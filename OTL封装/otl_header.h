#pragma once

#define OTL_ODBC
#define OTL_ANSI_CPP								// 中文支持
#define OTL_STREAM_READ_ITERATOR_ON					// image 等大量数据插入的支持
#define OTL_STL										// 支持用std::string 接受char* 类型

#define OTL_CPP_11_ON								// 支持连接池的 c++ 版本
#define OTL_CONNECT_POOL_ON							// 连接池支持
#define OTL_ANSI_CPP_11_VARIADIC_TEMPLATES    	 	// 支持 otl_value< >


#ifndef NOMINMAX									// std::max std::min  c++ 17 版本的一些问题
#define NOMINMAX
#endif

#ifndef _CRT_SECURE_NO_WARNINGS						// 函数安全
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <otl/otlv4.h>

// TODO: 其他平台测试 linux (Ubuntu )
#ifdef _WINDOWS
#pragma  comment(lib, "odbc32.lib")
#endif

#include <boost/date_time/posix_time/posix_time.hpp>
#include <json/json.h>
#include "loguru/loguru.hpp"

using OtlConnPool = otl_connect_pool<otl_connect, otl_exception>;

enum class EDBType
{
	dbINVALID = 0,                  // 无效数据源类型
	dbSQLSERVER = 1,                // SQLSERVER 数据库
	dbMYSSQL = 2,                   // MYSQL 数据库
	dbORACLE = 3                    // ORACLE 数据库
};

static otl_datetime otl_time_from_string(const std::string& str)
{
	otl_datetime odt;
	auto datetime = boost::posix_time::time_from_string(str);
	auto date = datetime.date();
	odt.year = date.year();
	odt.month = date.month();
	odt.day = date.day();
	auto time = datetime.time_of_day();
	odt.hour = time.hours();
	odt.minute = time.minutes();
	odt.second = time.seconds();
	odt.fraction = time.fractional_seconds();
	return odt;
}

static std::string otl_time_to_string(const otl_datetime& tm)
{
	char datetime_buff[32] = {};
	sprintf(datetime_buff, "%04d-%02d-%02d %02d:%02d", tm.year, tm.month, tm.day, tm.hour, tm.minute);

	return datetime_buff;
}

// longtext varchar(max) 等数据转字符串， 限定长度的 text 和 varchar 用 std::string 直接获取
#define OLT_LOB_STREAM_TO_STRING(lob, str) 				\
    while (!lob.eof())									\
    {													\
        std::string tmp;								\
        otl_long_string _sols;							\
        lob >> _sols;									\
        tmp.resize(_sols.len());						\
        memcpy(&tmp[0], _sols.v, _sols.len());			\
        str += tmp;										\
    }													\
    lob.close();


const static char* DB_TYPE_MSSQL_STR = "sqlserver";
const static char* DB_TYPE_MYSQL_STR = "mysql";
const static char* DB_TYPE_ORACLE_STR = "oracle";

static EDBType GetType(const std::string& desc)
{
	if (DB_TYPE_MSSQL_STR == desc)
	{
		return EDBType::dbSQLSERVER;
	}
	else if (DB_TYPE_MYSQL_STR == desc)
	{
		return EDBType::dbMYSSQL;
	}
	else if (DB_TYPE_ORACLE_STR == desc)
	{
		return EDBType::dbMYSSQL;
	}
	return EDBType::dbINVALID;
}

static std::string DBTypeToString(const EDBType& type)
{
	switch (type)
	{
	case EDBType::dbMYSSQL:
		return DB_TYPE_MSSQL_STR;
	case EDBType::dbSQLSERVER:
		return "sqlserver";
	case EDBType::dbORACLE:
		return "oracle";
	default:
		break;
	}
	return "";
}

const static char* OPT_JSON_ID = "id";
const static char* OPT_JSON_NAME = "name";
const static char* OPT_JSON_IP = "ip";
const static char* OPT_JSON_PORT = "port";
const static char* OPT_JSON_TYPE = "type";
const static char* OPT_JSON_DRIVER = "driver";
const static char* OPT_JSON_SCHEMA = "schema";
const static char* OPT_JSON_USER = "user";
const static char* OPT_JSON_PASSWORD = "password";
const static char* OPT_JSON_POOLNUM = "poolnum";


class OtlConnOption
{
public:
	OtlConnOption()
	{
		id = "";
		name = "";
		ip = "";
		port = -1;
		type = EDBType::dbSQLSERVER;
		driver = "";
		schema = "";
		user = "";
		password = "";
		m_pOtlConnStr = "";
		poolnum = 8;
	}

	std::string id;
	std::string name;
	std::string ip;
	int port;
	EDBType type;
	std::string driver;
	std::string schema;
	std::string user;
	std::string password;
	int poolnum;

	/// <summary>
	/// 生成对应的odbc链接串
	/// </summary>
	/// <param name="rebuild">是否重新生产连接串, 属性修改后需要设置true 使连接串更新</param>
	/// <returns></returns>
	std::string GetOtlConnString(const bool& rebuild = false)
	{
		if (rebuild || m_pOtlConnStr.empty())
		{
			switch (type)
			{
			case EDBType::dbMYSSQL:
				m_pOtlConnStr =
						"Driver={" + driver + "};Server=" + ip + ";Port=" + std::to_string(port) + ";Database=" +
						schema + ";User=" + user + ";Password=" + password + ";Option=3;";
				break;
			case EDBType::dbSQLSERVER:
				m_pOtlConnStr =
						"Driver={" + driver + "};Server=" + ip + ";Port:" + std::to_string(port) + ";Database=" +
						schema + ";UID=" + user + ";PWD=" + password + ";";
				break;
			case EDBType::dbORACLE:
				m_pOtlConnStr = "Driver={" + driver + "};Server=" + ip + ":" + std::to_string(port) + "/" + schema +
								";Uid=" + user + ";Pwd=" + password + ";";
				break;
			default:
				break;
			}
		}
		return m_pOtlConnStr;
	}

	OtlConnOption& operator=(const OtlConnOption& other)
	{
		this->type = other.type;
		this->name = other.name;
		this->ip = other.ip;
		this->port = other.port;
		this->driver = other.driver;
		this->schema = other.schema;
		this->user = other.user;
		this->password = other.password;
		this->poolnum = other.poolnum;

		return *this;
	}

	/*
	 {
		"type": "mysql",
		"ip": "192.168.0.154",
		"port": 3306,
		"driver": "Mysql odbc 8.0 ansi driver",
		"schema": "TZX_FloodDisaster_XJ",
		"user": "root",
		"password": "3edc9ijn~",
		"poolnum" : 10
	},

	*/

	bool FromString(const std::string& optConfig)
	{
		Json::Reader reader;
		Json::Value root;
		if (reader.parse(optConfig, root))
		{
			return FromJson(root);
		}
		return false;
	}

	/// <summary>
	/// 从固定格式的json加载对象
	/// </summary>
	/// <param name="root"></param>
	/// <returns></returns>
	bool FromJson(const Json::Value& root)
	{
		if (root.isMember("type"))
		{
			type = GetType(root["type"].asString());
		}
		else
		{
			return false;
		}
		if (root.isMember("name"))
		{
			name = root["name"].asString();
		}
		else
		{
			return false;
		}
		if (root.isMember("ip"))
		{
			ip = root["ip"].asString();
		}
		else
		{
			return false;
		}
		if (root.isMember("port"))
		{
			port = root["port"].asInt();
		}
		else
		{
			return false;
		}
		if (root.isMember("driver"))
		{
			driver = root["driver"].asString();
		}
		else
		{
			return false;
		}
		if (root.isMember("schema"))
		{
			schema = root["schema"].asString();
		}
		else
		{
			return false;
		}
		if (root.isMember("user"))
		{
			user = root["user"].asString();
		}
		else
		{
			return false;
		}
		if (root.isMember("password"))
		{
			password = root["password"].asString();
		}
		else
		{
			return false;
		}
		if (root.isMember("poolnum"))
		{
			poolnum = root["poolnum"].asInt();
		}
		else
		{
			poolnum = 10;
		}
		return true;
	}

	/// <summary>
	/// 转为固定形式的json
	/// </summary>
	/// <returns></returns>
	[[nodiscard]] Json::Value ToJson() const
	{
		Json::Value root;
		root[OPT_JSON_ID] = this->id;
		root[OPT_JSON_NAME] = this->name;
		root[OPT_JSON_TYPE] = DBTypeToString(this->type);
		root[OPT_JSON_IP] = this->ip;
		root[OPT_JSON_PORT] = this->port;
		root[OPT_JSON_DRIVER] = this->driver;
		root[OPT_JSON_SCHEMA] = this->schema;
		root[OPT_JSON_USER] = this->user;
		root[OPT_JSON_PASSWORD] = this->password;
		root[OPT_JSON_POOLNUM] = this->poolnum;

		return root;
	}

protected:
	std::string m_pOtlConnStr;

};

class OtlPoolBase
{
public:
	OtlPoolBase()
	{
		m_retry = 3;
		m_pConnPool = nullptr;
	}

	void SetConnOption(const OtlConnOption& option)
	{
		m_pConnOption = option;
	}

	bool Initialize(const OtlConnOption& option)
	{
		SetConnOption(option);
		return Initialize();
	}

	bool Initialize()
	{
		std::scoped_lock lock(m_pInitMutex);
		bool status = false;
		OtlConnOption tmpOption = m_pConnOption;
		std::string otlOdbcStr = tmpOption.GetOtlConnString(true);
		try
		{
			if (nullptr == m_pConnPool)
			{
				m_pConnPool = new OtlConnPool;
			}
			else  // 重新设置的情况
			{
				if (m_pConnPool->is_open())
				{
					m_pConnPool->close();
				}
			}
			// 连接池的数量不小于2
			m_pConnPool->open(otlOdbcStr.c_str(), std::max(2, tmpOption.poolnum), std::max(2, tmpOption.poolnum));
			status = true;
		}
		catch (otl_exception& e)
		{
			LOG_F(ERROR, "打开数据库连接: [%s] 失败, \n 错误消息:\n %s  \n 状态吗: %s \n 执行SQL: %s", otlOdbcStr.c_str(), e.msg, e.sqlstate, e.stm_text);
		}
		catch (std::exception& e)
		{
			LOG_F(ERROR, "%s", e.what());
		}
#ifndef NDEBUG
		LOG_F(INFO, "打开数据库连接: %s 成功.", otlOdbcStr.c_str());
#endif
		return status;
	}


	/// 获取连接池指针
	/// \return
	OtlConnPool* GetPool()
	{
		// TODO： 目前没有找到重连方式
//		std::scoped_lock lock(m_pGetMutex);
//		if (nullptr != m_pConnPool)
//		{
//
//			if (!m_pConnPool->is_open()) // 数据库重启或者网络断开连接 后 需要重新连接
//			{
//				int retry = m_retry;
//				while (retry)
//				{
//					if (!Initialize())
//					{
//						LOG_F(ERROR, "数据库连接断开, 尝试重连 %d/%d 次", retry, m_retry);
//					}
//					retry--;
//				}
//			}
//		}
		return m_pConnPool;
	}

	[[nodiscard]] EDBType GetDbType() const
	{
		return m_pConnOption.type;
	}

//public:
//	int m_retry;
private:
	OtlConnOption m_pConnOption;
	OtlConnPool* m_pConnPool;
	std::mutex m_pInitMutex;
	std::mutex m_pGetMutex;

};

// OTL数据库使用的示例代码

#if HAS_OLT_POOL_DEMO

OtlPoolBase otl_pool_demo;

// 构建连接池的示例
void  Demo_BuildConnPool()
{
	std::string  option_config = "{\n"
								 "\t\t\"type\": \"mysql\",\n"
								 "\t\t\"ip\": \"192.168.0.154\",\n"
								 "\t\t\"port\": 3306,\n"
								 "\t\t\"driver\": \"Mysql odbc 8.0 ansi driver\",\n"
								 "\t\t\"schema\": \"TZX_FloodDisaster_XJ\",\n"
								 "\t\t\"user\": \"root\",\n"
								 "\t\t\"password\": \"3edc9ijn~\",\n"
								 "\t\t\"poolnum\" : 10\n"
								 "\t}";
	OtlConnOption opt;
	assert(opt.FromString(option_config));
	assert(otl_pool_demo.Initialize(opt));

}

// 使用连接池 查询示例
const static char* OracleQuerySql = "select STCD, TM, INTV, DRP from ST_PPTN_R where TM = to_date('2000-01-01 00:00','yyyy-MM-dd HH24:mi')";
const static char* MysqlQuerySql = "select STCD, TM, INTV, DRP from ST_PPTN_R where TM = '2000-01-01 00:00'";
const static char* MssqlQuerySql = "select STCD, TM, INTV, DRP from ST_PPTN_R where TM = '2000-01-01 00:00'";

void Demo_Query()
{
	auto conn_ptr = otl_pool_demo.GetPool()->get();
	otl_stream query_stream;
	try{
		switch(otl_pool_demo.GetDbType())
		{
		case EDBType::dbSQLSERVER:
			query_stream.open(256, MssqlQuerySql, *conn_ptr);
			break;
		case EDBType::dbMYSSQL:
			query_stream.open(256, MysqlQuerySql, *conn_ptr);
			break;
		case EDBType::dbORACLE:
			query_stream.open(256, OracleQuerySql, *conn_ptr);
			break;
		default:
			break;
		}

#ifdef SHOW_AUTO_LOOP
		for (auto& qs: query_stream)
		{
			otl_value<std::string> otl_stcd;
			otl_value<otl_datetime> otl_tm;
			otl_value<int> otl_intv;
			otl_value<double> otl_drp;
			otl_read_row(qs, otl_stcd, otl_tm, otl_intv, otl_drp);
			char datetime_buff[32] = {};
			sprintf_s(datetime_buff, "%04d-%02d-%02d %02d:%02d", otl_tm.v.year, otl_tm.v.month, otl_tm.v.day, otl_tm.v.hour, otl_tm.v.minute);
			std::cout << "STCD: " << otl_stcd.v << " TM: " << datetime_buff << " INTV: " << otl_intv.v << " DRP: " << otl_drp.v << std::endl;
		}
#else
		while (!query_stream.eof())
		{
			otl_value<std::string> otl_stcd;
			otl_value<otl_datetime> otl_tm;
			otl_value<int> otl_intv;
			otl_value<double> otl_drp;
			otl_read_row(query_stream, otl_stcd, otl_tm, otl_intv, otl_drp);
			char datetime_buff[32] = {};
			sprintf_s(datetime_buff, "%04d-%02d-%02d %02d:%02d", otl_tm.v.year, otl_tm.v.month, otl_tm.v.day, otl_tm.v.hour, otl_tm.v.minute);
			std::cout << "STCD: " << otl_stcd.v << " TM: " << datetime_buff << " INTV: " << otl_intv.v << " DRP: " << otl_drp.v << std::endl;
		}
#endif
	}
	catch (otl_exception& e)
	{
		LOG_F(ERROR, "查询失败, \nsql:%s \nmessage:%s \n state:%s", e.stm_text, e.msg, e.sqlstate);
	}
	catch (const std::exception& p)
	{
		LOG_F(ERROR, "%s", p.what());
	}
	query_stream.close();
	otl_pool_demo.GetPool()->put(std::move(conn_ptr));
}

void Demo_Insert()
{
	auto conn_ptr = otl_pool_demo.GetPool()->get();
	otl_stream insert_stream;
	try{
		conn_ptr->direct_exec("delete from ST_PPTN_R  DEMO");
		conn_ptr->commit();
		insert_stream.open(256, "insert into ST_PPTN_R(STCD, TM, INTV, DRP) values (:f1<char[32]>, :f2<timestamp>, :f3<int>, :f4<double>);", *conn_ptr);

		struct PptnRecord {
			std::string stcd;
			std::string tm;
			int intv;
			double drp;
		};
		std::vector<PptnRecord> pptnRecords;
		for (auto& record: pptnRecords)
		{
			otl_value<std::string> otl_stcd(record.stcd);
			otl_value<otl_datetime> otl_tm(otl_time_from_string(record.tm));
			otl_value<int> otl_intv(record.intv);
			otl_value<double> otl_drp(record.drp);
			otl_write_row(insert_stream, otl_stcd, otl_tm, otl_intv, otl_drp);
			insert_stream.flush();
		}
	}
	catch (otl_exception& e)
	{
		conn_ptr->rollback();
		LOG_F(ERROR, "插入失败, \nsql:%s \nmessage:%s \n state:%s", e.stm_text, e.msg, e.sqlstate);
	}
	catch (const std::exception& p)
	{
		LOG_F(ERROR, "%s", p.what());
	}
	insert_stream.close();
	otl_pool_demo.GetPool()->put(std::move(conn_ptr));
}

#endif
