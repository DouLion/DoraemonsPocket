#pragma once
#include <iostream>
#include "ezotl.h"
#include <time.h>
#if _TRY_VLD   // windows 平台检查内存泄漏
#include <vld.h>
#endif
void MSSQL_TEST();
void MSSQL_SELECT_SPEED_TEST();		// SQL SERVER 多轮(100)查询,取平均数测试查询速度
void MSSQL_SELECT_SPEED();
int main()
{
	// MSSQL_TEST();
	MSSQL_SELECT_SPEED_TEST();

	return 0;
}

void MSSQL_SELECT_SPEED_TEST()
{
	int round_num = 100;
	time_t time1 = time(nullptr);
	for (int i = 0; i < round_num; ++i)
	{
		MSSQL_SELECT_SPEED();
	}
	time_t time2 = time(nullptr);
	std::cout << "Average time per-round: " << (time2 - time1) * 1000.0 / round_num << " ms." << std::endl;
	//1. windows 10, i7 11700T, Release 本地数据库测试结果, 获取622104(622K)条数据, 每轮用时约840ms,表中 1866312(1866K)条数据;SQL MANAGE STUDIO 查询时间约2s. 磁盘占用不明显.
	//2. windows 10, i7 11700T, Release 本地数据库测试结果, 获取622104(622K)条数据, 每轮用时约11510 ms,表中 63765660(63M)条数据;
	//   SQL MANAGE STUDIO 查询时间约12s. 数据库磁盘(SSD)读速度300 - 400 M/s.

	return;
}
void MSSQL_SELECT_SPEED()
{
	std::string conn_str = "driver=sql server;server=192.168.0.155;port=1433;uid=sa;pwd=3edc9ijn~;database=HEBEI_RES_MODEL";
	std::string sql_select_sql = "select PID, datediff( HOUR, '2022-08-10 20:00', TM ), DRP from ForeRainData_M_2 where TM > '2022-08-10 20:00' and TM <= '2022-08-11 20:00' ";
	ezotl::ezconntion conn;
	conn.connect(conn_str);
	ezotl::ez_row_vec datav;

	conn.select(sql_select_sql, datav);
	//1. windows 10, i7 11700T, Release 本地数据库测试结果, 获取622104条数据, 每轮至此用时约30ms,表中 1866312条数据;SQL MANAGE STUDIO 查询时间约2s.
	//2. windows 10, i7 11700T, Release 本地数据库测试结果, 获取622104条数据, 每轮用时约11510 ms,表中 63765660条数据;SQL MANAGE STUDIO 查询时间约12s.
	std::map<int, std::map<int, float>> retMap;
	for (auto data_row : datav)
	{
		int pid = data_row[0].GetInt32();
		int hourDiff = data_row[1].GetInt32();
		float drp = data_row[2].GetFloat();
		retMap[pid][hourDiff] = drp;
	}
	ezrow_vec_free(datav);
	conn.close();
}

void MSSQL_TEST()
{
	ezotl::ezconntion conn;
	std::string conn_str = "driver=sql server;server=.;uid=sa;pwd=3edc9ijn~;database=TestOtl;";
	conn.connect(conn_str);

	std::string sql_create_table = "CREATE TABLE TestOtl(a_bigint bigint NULL,b_binary_50 binary(50) NULL,c_bit bit NULL,d_char_10 char(10) NULL,e_date date NULL,f_datetime datetime NULL,g_datetime2_7 datetime2(7) NULL,h_datetimeoffset datetimeoffset(7) NULL,i_decimal decimal(18, 0) NULL,j_float float NULL,k_geography geography NULL,l_geometry geometry NULL,m_hierarchyid hierarchyid NULL,n_image image NULL,o_int int NULL,p_money money NULL,q_nchar_10 nchar(10) NOT NULL,r_ntext ntext NULL,s_numeric numeric(18, 0) NULL,t_nvarchar nvarchar(50) NULL,u_nvarchar nvarchar(max) NULL,v_real real NULL,w_smalldatetime smalldatetime NULL,x_smallint smallint NULL,y_smallmoney smallmoney NULL,a1_text text NULL,b1_time time(7) NULL,c1_timestamp timestamp NULL,d1_tinyint tinyint NULL,e1_uniqueidentifier uniqueidentifier NOT NULL,f1_varbinary varbinary(50) NULL,g1_varchar_50 varchar(50) NULL,h1_varchar_max varchar(max) NULL,j1_xml xml NULL, CONSTRAINT PK_TestOtl PRIMARY KEY CLUSTERED (e1_uniqueidentifier ASC)WITH(PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON[PRIMARY] )";

	std::string sql_direct_insert = "insert into OtlTypes (a_bigint,b_binary_50,c_bit,d_char_10,e_date,f_datetime,g_datetime2_7,h_datetimeoffset,i_decimal,j_float,m_hierarchyid,n_image,o_int,p_money,q_nchar_10,r_ntext,s_numeric,t_nvarchar,u_nvarchar,v_real,w_smalldatetime,x_smallint,y_smallmoney,a1_text,b1_time,c1_timestamp,d1_tinyint,e1_uniqueidentifier,f1_varbinary,g1_varchar_50,h1_varchar_max,j1_xml) values ('10', null, '5', '10字符','2022-03-06', '2022-03-06','2022-03-06', '2022-03-06',189,188, hierarchyID::Parse('/1/2.2/'), null,1024, 24, '13aab吸', null, 50, '中文', null, 18, '2022-03-06','222', 222, null, '2022', null, 222,  '7F9619FF-8B86-D011-B42D-00C04FC964FF', null, '50五十fifty', '50五十fifty50五十fifty50五十fifty','<Person><ID>1</ID><Name>刘备</Name></Person>');";

	std::string sql_statement_insert = "insert into OtlTypes (a_bigint,b_binary_50,c_bit,d_char_10,e_date,f_datetime,g_datetime2_7,h_datetimeoffset,i_decimal,j_float,m_hierarchyid,n_image,o_int,p_money,q_nchar_10,r_ntext,s_numeric,t_nvarchar,u_nvarchar,v_real,w_smalldatetime,x_smallint,y_smallmoney,a1_text,b1_time,c1_timestamp,d1_tinyint,e1_uniqueidentifier,f1_varbinary,g1_varchar_50,h1_varchar_max,j1_xml) values (:f1<double>, null, :f2<short>, :f3<char[10]>, :f4<char[40]>, :f5<timestamp>,:f4<char[40]>, :f4<char[40]>,:f1<double>,:f1<double>, null, null,:f1<int>, :f1<double>, :f3<char[10]>, null, :f1<double>, :f1<char[50]>, null, :f1<double>, :f5<timestamp>,:f2<short>, :f1<double>, null, :f4<char[40]>, null, :f2<short>,  :f3<raw>, null, :f3<raw>, :f3<raw>,:f1<varchar_long>)";


	std::string sql_query = "SELECT a_bigint,b_binary_50,c_bit,d_char_10,e_date,f_datetime,g_datetime2_7,h_datetimeoffset,i_decimal,j_float,m_hierarchyid,n_image,o_int,p_money,q_nchar_10,r_ntext,s_numeric,t_nvarchar,u_nvarchar,v_real,w_smalldatetime,x_smallint,y_smallmoney,a1_text,b1_time,c1_timestamp,d1_tinyint,e1_uniqueidentifier,f1_varbinary,g1_varchar_50,h1_varchar_max,j1_xml.value('(/Person/Name)[1]', 'nvarchar(10)') as pname FROM OtlTypes";
	ezotl::ez_row_vec datav;

	conn.select(sql_query, datav);
	ezrow_vec_free(datav);
	conn.close();

	return;
}

