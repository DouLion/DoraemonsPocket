/*
* @copyright: Beijing tianzhixiang Information Technology Co., Ltd. All rights reserved.
* @website: http://www.tianzhixiang.com.cn/
* @author: dou li yang
* @date: 2022/10/14 10:11
* @version: 1.0.1
* @description: û������������ ֧�����ĵ� otlv4 ��װ, ���� https://otl.sourceforge.net/otl3.htm#toc 20BIG �ڼ���Ҫ��ǽ
*/

#pragma once

/* TODO:
* 1. ��������select ����
* 2. insert bulk insert update  delete exec
* 3. ���ӳ�
* 4. ����һ��ezconntion ���� ͬʱ���ж�� insert select ʱ
*/

#define OTL_ODBC
#define OTL_ANSI_CPP							// ����֧��
#define OTL_STREAM_READ_ITERATOR_ON				// image �ȴ������ݲ����֧��	
#define OTL_STL									// std::string ֱ�ӽ��� char[N] ����

//#include <otl/otlv4.h>  // ���·������ vcpkg ��װ��otl ·��
#include "otlv4.h"
#include <iostream>
#include <vector>
#include <map>
#include <time.h>

#pragma  comment(lib, "odbc32.lib")

#define EZ_EXCEPTION_WRAPPER(_exec_func, _exec_state) {try { _exec_func; } catch (otl_exception& p) { std::cerr << p.msg << std::endl; std::cerr << "Info : " << p.sqlstate << std::endl; std::cerr << "Error Code : " << p.stm_text << std::endl; std::cerr << "SQL : " << p.var_info << std::endl; _exec_state = false;} _exec_state = true;}

// inline

// Deprecated: ���Բ�ͬ ���ݿ� �� otl ��Ӧ������
void print_otl_type_name(const int& otlType) {
	switch (otlType)
	{
	case otl_var_char:
		std::cout << "\t char" << std::endl;
		break;
	case otl_var_double:
		std::cout << "\t double" << std::endl;
		break;
	case otl_var_float:
		std::cout << "\t float" << std::endl;
		break;
	case otl_var_int:
		std::cout << "\t int " << std::endl;
		break;
	case otl_var_unsigned_int:
		std::cout << "\t _unsigned_int" << std::endl;
		break;
	case otl_var_short:
		std::cout << "\t short" << std::endl;
		break;
	case otl_var_long_int:
		std::cout << "\t long_int" << std::endl;
		break;
	case otl_var_timestamp:
		std::cout << "\t timestamp" << std::endl;
		break;
	case otl_var_varchar_long:
		std::cout << "\t varchar_long" << std::endl;
		break;
	case otl_var_raw_long:
		std::cout << "\t raw_long" << std::endl;
		break;
	case otl_var_clob:
		std::cout << "\t clob" << std::endl;
		break;
	case otl_var_blob:
		std::cout << "\t blob" << std::endl;
		break;
	case otl_var_refcur:
		std::cout << "\t refcur" << std::endl;
		break;
	case otl_var_long_string:
		std::cout << "\t long_string" << std::endl;
		break;
	case otl_var_db2time:
		std::cout << "\t db2time" << std::endl;
		break;
	case otl_var_db2date:
		std::cout << "\t db2date" << std::endl;
		break;
	case otl_var_tz_timestamp:
		std::cout << "\t tz_timestamp" << std::endl;
		break;
	case otl_var_ltz_timestamp:
		std::cout << "\t ltz_timestamp" << std::endl;
		break;
	case otl_var_bigint:
		std::cout << "\t bigint" << std::endl;
		break;
	case otl_var_raw:
		std::cout << "\t raw" << std::endl;
		break;
	default:
		std::cout << "\t none" << std::endl;
		break;
	}
}


namespace ezotl // easy otl
{
	enum class EDbType { dtMSSQL = 0, dtMYSQL = 1, dtORACLE = 2, dtSQLITE = 3, dtPOSTGRESQL = 4, dtDEFAULT = 0 };
	// û��, ������Ϊ ���ݿ����� �� otl���͵Ĳ�ѯ����
	enum class EColumType
	{
		ctBIGINT = 0,  // ���� bigint
		/* ��Ӧ����
		MSSQL :  unsigned 64-bit int
		MYSQL : unsigned 64-bit int
		ORCLE : unsigned 64-bit int
		SQLITE : unsigned 64-bit int  ..

		*/
		ctBLOB = 1, // ���� blob
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :  Oracle 8 and higher; BLOB
		SQLITE :
		*/
		ctBDOUBLE = 2, // ���� bdouble
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		POSTGRESQL: Should only be used when PL/SQL tables of type BINARY_DOUBLE are used.
		*/
		ctBFLOAT = 3, // ���� bfloat
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		POSTGRESQL : Should only be used when PL/SQL tables of type BINARY_FLOAT are used.
		*/
		ctCHAR = 4, // ���� char  ʾ�� :f1<char[����]> ���� OTL 4.0.118 ���߰汾 :f1<char(����)>  ����ָ������
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctCHARZ = 5, // ���� charz
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		POSTGRESQL Should be used only when [PL/SQL tables] of type CHAR(XXX) are used.
		*/
		ctCLOB = 6, // ���� clob
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE : for Oracle 8 and higher: CLOB, NCLOB
		SQLITE :
		*/
		ctDB2DATE = 7, // ���� db2date
		/* ��Ӧ����
		MSSQL :
		MYSQL : DB2 DATEs
		ORCLE :
		SQLITE :
		*/
		ctDB2TIME = 8, // ���� db2time
		/* ��Ӧ����
		MSSQL :
		MYSQL :  DB2 TIMEs
		ORCLE :
		SQLITE :
		*/
		ctDOUBLE = 9, // ���� ctdouble : 8-byte floating point number
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctFLOAT = 10, // ���� ctfloat : 4-byte floating point number
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctINT = 11, // ���� ctint : signed 32-bit int
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctLONG = 12, // ���� ctlong
		ctLTZ_TIMESTAMP = 13, // ���� bigint
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE : Oracle 9i TIMESTAMP WITH LOCAL TIME ZONE , with #define [OTL_ORA_TIMESTAMP], and [otl_datetime]
		SQLITE :
		*/
		ctNCHAR = 14, // ���� ctnchar
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctNCLOB = 15, // ���� ctnclob
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctRAW = 16, // ���� ctraw
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctRAW_LONG = 17, // ���� ctraw_long
		/* ��Ӧ����
		ODBC: SQL_LONGVARBINARY, SQL_VARBINARY
		MSSQL : image
		MYSQL :  BLOB
		ORCLE : RAW, LONG RAW
		SQLITE :
		*/
		ctREFCUR = 18, // ���� ctrefcur
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctSHORT = 19, // ���� ctshort
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctSDO_GEOMETRY = 20, // ���� ctsdo_geometry
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctTIMESTAMP = 21, // ���� cttimestamp
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctTZ_TIMESTAMP = 22, // ���� cttz_timestamp
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctUBIGINT = 23, // ���� ctubigint
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctUNSIGNED = 24, // ���� ctunsigned
		/* ��Ӧ����
		MSSQL :
		MYSQL :
		ORCLE :
		SQLITE :
		*/
		ctVARCHAR_LONG = 25 // ���� ctvarchar_long
		/* ��Ӧ����
		MSSQL : TEXT
		MYSQL :
		ORCLE :
		SQLITE :
		*/
	};
	enum class EStreamMode { smNormal = 0, smBlob = 1 };
	struct ezvar
	{
		ezvar() {
			size = 0;
			v = nullptr;
			// type = otl_var_none;
		};
		ezvar(int _size) {
			size = _size;
			v = nullptr;
			if (size != 0)
			{
				ezalloc();
			}

		}
		void* v;
		std::uint64_t size;
		otl_var_enum type;

		std::string GetString()
		{
			return std::string((char*)v);
		}

		unsigned char* GetBytes(std::uint64_t& nums)
		{
			nums = size;
			return (unsigned char*)v;
		}

		// short 
		short GetInt16()
		{
			return ((short*)v)[0];
		}

		short* GetInt16(int& nums)
		{
			nums = size / sizeof(short);
			return (short*)v;
		}

		void SetInt16(const short& val)
		{
			size = sizeof(short);
			ezalloc();
			memcpy(v, &val, size);
		}

		void SetInt16(const short* val, const int& nums)
		{
			size = sizeof(short) * nums;
			ezalloc();
			memcpy(v, val, size);
		}

		// int
		int  GetInt32()
		{
			return ((int*)v)[0];
		}

		int* GetInt32(int& nums)
		{
			nums = size / sizeof(int);
			return (int*)v;
		}

		void SetInt32(const int& val)
		{
			size = sizeof(int);
			ezalloc();
			memcpy(v, &val, size);
		}

		void SetInt32(const int* val, const int& nums)
		{
			size = sizeof(int) * nums;
			ezalloc();
			memcpy(v, val, size);
		}

		// long
		long long GetInt64()
		{
			return ((int*)v)[0];
		}

		long long* GetInt64(int& nums)
		{
			nums = size / sizeof(long long);
			return (long long*)v;
		}

		void SetInt64(const long long& val)
		{
			size = sizeof(long long);
			ezalloc();
			memcpy(v, &val, size);
		}

		void SetInt64(const long long* val, const int& nums)
		{
			size = sizeof(long long) * nums;
			ezalloc();
			memcpy(v, val, size);
		}

		// float
		float GetFloat()
		{
			return ((float*)v)[0];
		}

		float* GetFloat(int& nums)
		{
			nums = size / sizeof(float);
			return (float*)v;
		}

		void SetFloat(const float& val)
		{
			size = sizeof(float);
			ezalloc();
			memcpy(v, &val, size);
		}

		void SetFloat(const float* val, const int& nums)
		{
			size = sizeof(float) * nums;
			ezalloc();
			memcpy(v, val, size);
		}

		// double 
		double GetDouble()
		{
			return ((double*)v)[0];
		}

		double* GetDouble(int& nums)
		{
			nums = size / sizeof(double);
			return (double*)v;
		}

		void SetDouble(const double& val)
		{
			size = sizeof(double);
			ezalloc();
			memcpy(v, &val, size);
		}

		void SetDouble(const double* val, const int& nums)
		{
			size = sizeof(double) * nums;
			ezalloc();
			memcpy(v, val, size);
		}


		struct tm GetDateTime()
		{
			struct tm tmp;
			memcpy(&tmp, v, sizeof(struct tm));
			return tmp;
		}

		void SetDateTime(const struct tm& tmp)
		{
			size = sizeof(tmp);
			ezalloc();
			memcpy(v, &tmp, size);
		}


		void SetDateTime(const std::string& timeStr)
		{
			// TODO: 
		}

		void ezalloc()
		{
			v = malloc(size * sizeof(char));
		}

		void ezfree()
		{
			if (nullptr != v)
			{
				free(v); v = nullptr;
				size = 0;
			}
		}
	};
	typedef std::vector<ezvar> ezrowv;
	typedef std::vector<std::vector<ezvar>> ez_row_vec;
	struct ez_row_map
	{
		ezvar GetData(const std::string& colName, const int& rowIdx)
		{
			if (colIndex.find(colName) != colIndex.end() && rowIdx < data.size())
			{
				return data[rowIdx][colIndex[colName]];
			}
		}
		std::map<std::string, int> colIndex;
		ez_row_vec data;
	};
	static void ezrow_vec_free(const ez_row_vec& ezrv)
	{
		for (auto ezr : ezrv)
		{
			for (auto ezv : ezr)
			{
				ezv.ezfree();
			}
		}
	}
	class ezconntion {
	public:
		ezconntion() {
			// ��ʼ�� otl����
			otl_connect::otl_initialize();
			m_DbType = EDbType::dtMSSQL;
		}

		bool connect(const std::string& connStr, const EDbType& dbType = EDbType::dtMSSQL)
		{
			bool state = false;
			m_DbType = dbType;
			EZ_EXCEPTION_WRAPPER(m_db.rlogon(connStr.c_str()), state);
			return state;
		}

		void _exec(const std::string& sql, long& affectRows)
		{
			affectRows = otl_cursor::direct_exec(m_db, sql.c_str(), otl_exception::enabled);
		}

		long exec(const std::string& sql)
		{
			int affect_rows = 0;
			long rowId = 0;
			bool state = false;
			EZ_EXCEPTION_WRAPPER(_exec(sql, rowId), state);
			return affect_rows;
		}
		/*!
		* @desc: ����һЩ����
		* @param[sql]: ����ִ�е�sql
		* @param[data]: statement ׼��������
		* @param[affect_rows]: ��Ӱ�������
		* @param[rowID]: ��ʱ����,Ŀǰ��֧��ORACLE�Ĳ���ʱ��ȡ����RowID, ����ͨ����ͬSQL��ѯ����RowID������ȡ
		* @param[mode]: 
		*/
		void _insert(const std::string& sql, const ez_row_vec& data, long& affect_rows, long& rowId, const EStreamMode& mode = EStreamMode::smNormal)
		{
			otl_stream _i_stream;
			if (EStreamMode::smNormal != mode)
			{
				_i_stream.set_lob_stream_mode(true);
				_i_stream.open(1, sql.c_str(), m_db);
			}
			else
			{
				_i_stream.open(50, sql.c_str(), m_db);
			}
			if (!data.empty())
			{
				int colnum = 0;
				otl_var_desc* cdesc = _i_stream.describe_in_vars(colnum);

				if (data.at(0).size() != colnum)  // ��������к�.. �Ƿ�һ��
				{
					_i_stream.close();
					affect_rows = 0;
					rowId = 0;
					return;
				}
				for (auto rdata : data)
				{
					for (int i = 0; i < colnum; ++i)
					{
						if (otl_var_char == cdesc[i].ftype)
						{
						}
						else if (otl_var_raw_long == cdesc[i].ftype)
						{
						}
						else if (otl_var_int == cdesc[i].ftype)
						{
						}
					}
				}
			}
			affect_rows = _i_stream.get_rpc();
			// otl_subscriber::subscribe
		}

		long insert(const std::string& sql, const ez_row_vec& data, const EStreamMode& mode = EStreamMode::smNormal)
		{
			long affect_rows = 0;
			long rowId = 0;
			bool state = false;
			EZ_EXCEPTION_WRAPPER(_insert(sql, data, affect_rows, rowId, mode), state);
			return affect_rows;
		}

		/*!
		* @desc: ��� MSSQL �� MYSQL �Ĵ������ݲ����ʵ��, SQL SEVER ����Ҫ���� ���ش洢�ļ�+�����ļ�(�������벿����ʱ��Ҫ),
				 ���ҷǱ���������Ҫʹ�ù����ļ��еķ�ʽ,ʹĿ��������Է��ʵ������ļ�.
		* @param[sql]: MYSQL -- Insert into table (col1, col2, col3) values ('v11','v12', 'v13' ) , ('v21','v22', 'v23' ) .....
		*			   SQLSERVER -- BULK INSERT ForeRainPoint_B FROM '' WITH (FORMATFILE = 'format_file_path' ); �����ļ�
		*			   ����  BULK INSERT ForeRainPoint_B FROM 'filename' WITH (FIELDTERMINATOR = ',',ROWTERMINATOR = '0x0a'); �м�			 ���ŷָ��� ĩβ \n
		* @param[data]:
		* @param[affectiveRows]:
		* @param[affectRows]:
		* @param[lastRowID]:
		* @mode[]:
		* @return:
		*/
		void _bulk_insert(const std::string& sql, const ez_row_vec& data, int& affectRows, int& lastRowID, const EStreamMode& mode = EStreamMode::smNormal)
		{
			if (EDbType::dtMSSQL == m_DbType)
			{
				// 1. ���sql�е������ļ���¼·��
				// 2. ������д���ļ�
				// 3. ִ�� bulk insert ����
			}
			else if (EDbType::dtMYSQL == m_DbType)
			{
				// 1. ƴ�Ӳ���sql
				// 2. ִ��bulk insert ���� 
				std::string _bulkInsertSql = sql + " values ";
				for (auto _r_data : data)
				{
					// for

				}
			}
			else
			{
				std::cout << "Not implement besides connection type of MS SQL SEVER and MSSQL" << std::endl;
				// ��֧�ֵķ�ʽֱ������ͨ���뷽ʽ

			}

		}

		void bulk_insert(const std::string& sql, const ez_row_vec& data, const EStreamMode& mode = EStreamMode::smNormal)
		{

		}



		/// <summary>
		/// 
		/// </summary>
		/// <param name="sql">��ѯʹ�õ�sql, </param>
		/// <param name="data"></param>
		/// <param name="colIndex"></param>
		/// <param name="mode"></param>
		void _select(const std::string& sql, ez_row_vec& data, std::map<std::string, int>& colIndex, const EStreamMode& mode = EStreamMode::smNormal)
		{
			otl_stream _s_stream;
			if (EStreamMode::smNormal != mode)
			{
				_s_stream.set_lob_stream_mode(true);
				_s_stream.open(1, sql.c_str(), m_db);
			}
			else
			{
				_s_stream.open(50, sql.c_str(), m_db);
			}
			int colnum = 0;
			otl_column_desc* cdesc = _s_stream.describe_select(colnum);
			for (int i = 0; i < colnum; ++i)
			{
				if (strlen(cdesc[i].name) == 0)
				{
					colIndex.insert({ std::to_string(i), i });
				}
				else
				{
					colIndex.insert({ cdesc[i].name, i });
				}
			}
			while (!_s_stream.eof())
			{
				ezrowv _row;
				for (int c = 0; c < colnum; ++c)
				{
					ezvar _tv(cdesc[c].dbsize);
					if (otl_var_char == cdesc[c].otl_var_dbtype || otl_var_raw == cdesc[c].otl_var_dbtype)
					{
						std::string s; _s_stream >> s;
						if (_s_stream.is_null())
						{
							_row.push_back(ezvar());
							_tv.ezfree();
							continue;
						}
						memcpy(_tv.v, &s[0], _tv.size);
					}
					else if (otl_var_raw_long == cdesc[c].otl_var_dbtype || otl_var_varchar_long == cdesc[c].otl_var_dbtype)
					{
						otl_lob_stream lob;
						_s_stream >> lob;
						int vv = lob.len();
						if (!lob.len())
						{
							// std::cout << "NULL" << std::endl;
							_row.push_back(ezvar());
							_tv.ezfree();
							continue;
						}
						std::uint64_t size = 0;
						std::vector<otl_long_string> olsv;
						while (!lob.eof())
						{
							otl_long_string _sols;
							lob >> _sols;
							size += _sols.len();
							olsv.push_back(_sols);
						}
						lob.close();
						_tv.ezfree();
						_tv.size = size;
						_tv.ezalloc();
						unsigned char* _cur = (unsigned char*)_tv.v;
						for (auto _ols : olsv)
						{
							memcpy(_cur, _ols.v, _ols.len());
							_cur += _ols.len();
						}
					}
					else if (otl_var_int == cdesc[c].otl_var_dbtype)
					{
						int ii = 0; _s_stream >> ii;
						memcpy(_tv.v, &ii, _tv.size);
					}
					else if (otl_var_unsigned_int == cdesc[c].otl_var_dbtype)
					{
						unsigned int ii = 0; _s_stream >> ii;
						memcpy(_tv.v, &ii, _tv.size);
					}
					else if (otl_var_short == cdesc[c].otl_var_dbtype)
					{
						short int ii = 0; _s_stream >> ii;
						memcpy(_tv.v, &ii, _tv.size);
					}
					else if (otl_var_double == cdesc[c].otl_var_dbtype)
					{
						double ii = 0; _s_stream >> ii;
						memcpy(_tv.v, &ii, _tv.size);
					}
					else if (otl_var_float == cdesc[c].otl_var_dbtype)
					{
						float ii = 0; _s_stream >> ii;
						memcpy(_tv.v, &ii, _tv.size);
					}
					else if (otl_var_timestamp == cdesc[c].otl_var_dbtype)
					{
						otl_datetime _date_time;
						_s_stream >> _date_time;
						_tv.ezfree();
						struct tm tmp_tm;
						tmp_tm.tm_year = _date_time.year;
						tmp_tm.tm_mon = _date_time.month;
						tmp_tm.tm_mday = _date_time.day;
						tmp_tm.tm_hour = _date_time.hour;
						tmp_tm.tm_min = _date_time.minute;
						tmp_tm.tm_sec = _date_time.second;
						_tv.size = sizeof(tmp_tm);
						_tv.ezalloc();
						memcpy(_tv.v, &tmp_tm, _tv.size);
					}
					else
					{
						std::cerr << "Not known type." << std::endl;
					}

					_row.push_back(_tv);
				}
				data.push_back(_row);
			}
			_s_stream.close();
		}

		/*!
		* @desc : �����ݿ��в�ѯ����
		* @param[sql]: ��ִ�е�SQL
		* @param[data]: ��ѯ��������
		* @param[mode]: otl_stream ��ģʽ smNormal: ��ͨģʽ, smBlob ���⴦����ļ�������������� ; Ĭ��һֱ���� δȷ��������ɵ�Ӱ��
		* @return: ��������
		*/
		long select(const std::string& sql, ez_row_vec& data, const EStreamMode& mode = EStreamMode::smBlob)
		{
			std::map<std::string, int> colIndex;
			bool state = false;
			EZ_EXCEPTION_WRAPPER(_select(sql, data, colIndex, mode), state);
			if (state)
			{
				return data.size();
			}
			return 0;
		}

		long select(const std::string& sql, ez_row_map& data, const EStreamMode& mode = EStreamMode::smBlob)
		{
			std::map<std::string, int> colIndex;
			std::vector<ezrowv> alldata;
			bool state = false;
			EZ_EXCEPTION_WRAPPER(_select(sql, alldata, colIndex, mode), state);
			if (state)
			{
				data.colIndex = colIndex;
				data.data = alldata;
				return data.data.size();
			}
			return 0;
		}

		bool close()
		{
			bool state = false;
			EZ_EXCEPTION_WRAPPER(m_db.logoff(), state);
			return state;
		}

	public:

		otl_connect m_db;
		EDbType m_DbType;
	};


}