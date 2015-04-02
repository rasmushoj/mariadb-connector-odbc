/*
  Copyright (c) 2001, 2012, Oracle and/or its affiliates. All rights reserved.
                2013 MontyProgram AB

  The MySQL Connector/ODBC is licensed under the terms of the GPLv2
  <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
  MySQL Connectors. There are special exceptions to the terms and
  conditions of the GPLv2 as it is applied to this software, see the
  FLOSS License Exception
  <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "tap.h"

ODBC_TEST(t_odbc3_error)
{
  SQLHENV henv1;
  SQLHDBC Connection1;
  SQLHSTMT Stmt1;
  SQLINTEGER ov_version;

  CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
  CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)SQL_OV_ODBC3, 0));

  CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));

  CHECK_ENV_RC(henv1, SQLGetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)&ov_version, 0, 0));
  is_num(ov_version, SQL_OV_ODBC3);

  CHECK_DBC_RC(Connection1, SQLConnect(Connection1, my_dsn, SQL_NTS, my_uid, SQL_NTS,
                           my_pwd, SQL_NTS));

  CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

  ERR_SIMPLE_STMT(Stmt1, "SELECT * FROM non_existing_table");
  if (check_sqlstate(Stmt1, "42S02") != OK)
    return FAIL;

  OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS t_error");
  OK_SIMPLE_STMT(Stmt1, "CREATE TABLE t_error (id INT)");

  ERR_SIMPLE_STMT(Stmt1, "CREATE TABLE t_error (id INT)");
  if (check_sqlstate(Stmt1, "42S01") != OK)
    return FAIL;

  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));

  FAIL_IF(SQLSetStmtAttr(Stmt1, SQL_ATTR_FETCH_BOOKMARK_PTR,
                                     (SQLPOINTER)NULL, 0) !=SQL_ERROR, "Error expected");
  if (check_sqlstate(Stmt1, "HYC00") != OK)
    return FAIL;

  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS t_error");

  CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));

  CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));

  CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

  return OK;
}


ODBC_TEST(t_odbc2_error)
{
  SQLHENV henv1;
  SQLHDBC Connection1;
  SQLHSTMT Stmt1;
  SQLINTEGER ov_version;

  CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
  CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)SQL_OV_ODBC2, 0));

  CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));

  CHECK_ENV_RC(henv1, SQLGetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)&ov_version, 0, 0));
  is_num(ov_version, SQL_OV_ODBC2);

  CHECK_DBC_RC(Connection1, SQLConnect(Connection1, my_dsn, SQL_NTS, my_uid, SQL_NTS,
                           my_pwd, SQL_NTS));

  FAIL_IF(SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1) == SQL_ERROR, "Success expected");

  ERR_SIMPLE_STMT(Stmt1, "SELECT * FROM non_existing_table");
  if (check_sqlstate(Stmt1, "S0002") != OK)
    return FAIL;

  OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS t_error");
  OK_SIMPLE_STMT(Stmt1, "CREATE TABLE t_error (id INT)");

  ERR_SIMPLE_STMT(Stmt1, "CREATE TABLE t_error (id INT)");
  if (check_sqlstate(Stmt1, "S0001") != OK)
    return FAIL;

  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));

  FAIL_IF(SQLSetStmtAttr(Stmt1, SQL_ATTR_ENABLE_AUTO_IPD,
                                     (SQLPOINTER)SQL_TRUE, 0) !=SQL_ERROR,
                                     "Error expected");
  if (check_sqlstate(Stmt1, "S1C00") != OK)
    return FAIL;

  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS t_error");

  CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));

  CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));

  CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

  return OK;
}


ODBC_TEST(t_diagrec)
{
  SQLCHAR   sqlstate[6]= {0};
  SQLCHAR   message[255]= {0};
  SQLINTEGER native_err= 0;
  SQLSMALLINT msglen= 0;

  ERR_SIMPLE_STMT(Stmt, "DROP TABLE t_odbc3_non_existent_table");

#if UNIXODBC_BUG_FIXED
  /*
   This should report no data found, but unixODBC doesn't even pass this
   down to the driver.
  */
  FAIL_IF(SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 2, sqlstate,
                                   &native_err, message, 0, &msglen),
              SQL_NO_DATA_FOUND);
#endif

  CHECK_STMT_RC(Stmt, SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, sqlstate,
                               &native_err, message, 255, &msglen));

  /* MSSQL returns SQL_SUCCESS in similar situations */
  FAIL_IF(SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, sqlstate,
                                   &native_err, message, 0, &msglen) != SQL_SUCCESS, "Success expected");

  FAIL_IF(SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, sqlstate,
                                   &native_err, message, 10, &msglen) != SQL_SUCCESS_WITH_INFO, "swi expected");

  FAIL_IF(SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, sqlstate,
                                   &native_err, message, -1, &msglen) != SQL_ERROR, "Error expected");


  return OK;
}


ODBC_TEST(t_warning)
{
  SQLCHAR    szData[20];
  SQLLEN     pcbValue;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_warning");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_warning (col2 CHAR(20))");
  OK_SIMPLE_STMT(Stmt, "INSERT INTO t_warning VALUES ('Venu Anuganti')");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CONCURRENCY,
                                (SQLPOINTER)SQL_CONCUR_ROWVER, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, 0));

  /* ignore all columns */
  OK_SIMPLE_STMT(Stmt, "SELECT * FROM t_warning");

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  FAIL_IF(SQLGetData(Stmt, 1, SQL_C_CHAR, szData, 4, &pcbValue) != SQL_SUCCESS_WITH_INFO, "SWI expected");
  IS_STR(szData, "Ven", 3);
  is_num(pcbValue, 13);

  FAIL_IF(SQLGetData(Stmt, 1, SQL_C_CHAR, szData, 4, &pcbValue) != SQL_SUCCESS_WITH_INFO, "swi expected");
  IS_STR(szData, "u A", 3);
  is_num(pcbValue, 10);

  FAIL_IF(SQLGetData(Stmt, 1, SQL_C_CHAR, szData, 4, &pcbValue) != SQL_SUCCESS_WITH_INFO, "swi expected");
  IS_STR(szData, "nug", 3);
  is_num(pcbValue, 7);

  FAIL_IF(SQLGetData(Stmt, 1, SQL_C_CHAR, szData, 4, &pcbValue) != SQL_SUCCESS_WITH_INFO, "swi expected");
  IS_STR(szData, "ant", 3);
  is_num(pcbValue, 4);

  FAIL_IF(SQLGetData(Stmt, 1, SQL_C_CHAR, szData, 4, &pcbValue) != SQL_SUCCESS, "success expected");
  IS_STR(szData, "i", 1);
  is_num(pcbValue, 1);

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "eof expected");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_warning");

  return OK;
}


ODBC_TEST(t_bug3456)
{
  SQLINTEGER connection_id;
  char buf[100];
  SQLHENV henv2;
  SQLHDBC  Connection2;
  SQLHSTMT Stmt2;

  /* Create a new connection that we deliberately will kill */
  ODBC_Connect(&henv2, &Connection2, &Stmt2);
  OK_SIMPLE_STMT(Stmt2, "SELECT connection_id()");
  CHECK_STMT_RC(Stmt2, SQLFetch(Stmt2));
  connection_id= my_fetch_int(Stmt2, 1);
  CHECK_STMT_RC(Stmt2, SQLFreeStmt(Stmt2, SQL_CLOSE));

  /* From another connection, kill the connection created above */
  sprintf(buf, "KILL %d", connection_id);
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, (SQLCHAR *)buf, SQL_NTS));

  /* Now check that the connection killed returns the right SQLSTATE */
  ERR_SIMPLE_STMT(Stmt2, "SELECT connection_id()");

  return check_sqlstate(Stmt2, "08S01");
}


/*
 * Bug #16224: Calling SQLGetDiagField with RecNumber 0,DiagIdentifier
 *             NOT 0 returns SQL_ERROR
 */
ODBC_TEST(t_bug16224)
{
  SQLINTEGER diagcnt;

  ERR_SIMPLE_STMT(Stmt, "This is an invalid Query! (odbc test)");

  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 0,
                                 SQL_DIAG_NUMBER, &diagcnt,
                                 SQL_IS_INTEGER, NULL));
  is_num(diagcnt, 1);

  return OK;
}


/*
 * Test that binding invalid column returns the appropriate error
 */
ODBC_TEST(bind_invalidcol)
{
  SQLCHAR dummy[10];
  OK_SIMPLE_STMT(Stmt, "select 1,2,3,4");

  /* test out of range column number */
  FAIL_IF(SQLBindCol(Stmt, 10, SQL_C_CHAR, "", 4, NULL) != SQL_ERROR, "Error expected");
  is_num(check_sqlstate(Stmt, "07009"), OK);

  /* test (unsupported) bookmark column number */
  FAIL_IF(SQLBindCol(Stmt, 0, SQL_C_BOOKMARK, "", 4, NULL) != SQL_ERROR, "Error expected");

  is_num(check_sqlstate(Stmt, "07009"), OK);

  /* SQLDescribeCol() */
  FAIL_IF(SQLDescribeCol(Stmt, 0, dummy, sizeof(dummy), NULL, NULL,
                                    NULL, NULL, NULL) != SQL_ERROR, "SQL_ERROR expected");
  /* Older versions of iODBC return the wrong result (S1002) here. */
  FAIL_IF(check_sqlstate(Stmt, "07009") != OK &&
     check_sqlstate(Stmt, "S1002") != OK, "expected sqlstate 07009 or S1002");

  FAIL_IF(SQLDescribeCol(Stmt, 5, dummy, sizeof(dummy), NULL,
                                    NULL, NULL, NULL, NULL)!= SQL_ERROR, "Error expected");
  is_num(check_sqlstate(Stmt, "07009"), OK);

  /* SQLColAttribute() */
  FAIL_IF(SQLColAttribute(Stmt, 0, SQL_DESC_NAME, NULL, 0,
                                     NULL, NULL) != SQL_ERROR, "Error expected");
  is_num(check_sqlstate(Stmt, "07009"), OK);

  FAIL_IF(SQLColAttribute(Stmt, 7, SQL_DESC_NAME, NULL, 0,
                                     NULL, NULL) != SQL_ERROR, "Error expected");
  is_num(check_sqlstate(Stmt, "07009"), OK);

  return OK;
}


/*
 * Test the error given when not enough params are bound to execute
 * the statement.
 */
ODBC_TEST(bind_notenoughparam1)
{
  SQLINTEGER i= 0;
  CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *)"select ?, ?", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,
                                  SQL_INTEGER, 0, 0, &i, 0, NULL));
  FAIL_IF(SQLExecute(Stmt)!= SQL_ERROR, "error expected");
  return check_sqlstate(Stmt, "07002");
}


/*
 * Test the error given when not enough params are bound to execute
 * the statement, also given that a pre-execute happens (due to calling
 * SQLNumResultCols).
 */
ODBC_TEST(bind_notenoughparam2)
{
  SQLINTEGER i= 0;
  SQLSMALLINT cols= 0;
  CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *)"select ?, ?", SQL_NTS));

  /* trigger pre-execute */
  CHECK_STMT_RC(Stmt, SQLNumResultCols(Stmt, &cols));
  is_num(cols, 2);

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,
                                  SQL_INTEGER, 0, 0, &i, 0, NULL));
  FAIL_IF(SQLExecute(Stmt)!= SQL_ERROR, "error expected");
  return check_sqlstate(Stmt, "07002");
}


/*
 * Test that calling SQLGetData() without a nullind ptr
 * when the data is null returns an error.
 */
ODBC_TEST(getdata_need_nullind)
{
  SQLINTEGER i;
  OK_SIMPLE_STMT(Stmt, "select 1 as i , null as j ");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  
  /* that that nullind ptr is ok when data isn't null */
  CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_LONG, &i, 0, NULL));

  /* now that it's an error */
  FAIL_IF(SQLGetData(Stmt, 2, SQL_C_LONG, &i, 0, NULL) != SQL_ERROR, "Error expected");
  return check_sqlstate(Stmt, "22002");
}


/*
   Handle-specific tests for env and dbc diagnostics
*/
ODBC_TEST(t_handle_err)
{
  SQLHANDLE henv1, Connection1;

  CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv1));
  CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER));
  CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));

  /* we have to connect for the DM to pass the calls to the driver */
  CHECK_DBC_RC(Connection1, SQLConnect(Connection1, my_dsn, SQL_NTS, my_uid, SQL_NTS,
                           my_pwd, SQL_NTS));

  FAIL_IF( SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                  (SQLPOINTER)SQL_OV_ODBC3, 0)!= SQL_ERROR, "Error expected");
  is_num(check_sqlstate_ex(henv1, SQL_HANDLE_ENV, "HY010"), OK);

  FAIL_IF(SQLSetConnectAttr(Connection1, SQL_ATTR_ASYNC_ENABLE,
                                      (SQLPOINTER)SQL_ASYNC_ENABLE_ON,
                                      SQL_IS_INTEGER) != SQL_SUCCESS_WITH_INFO, "swi expected");
  is_num(check_sqlstate_ex(Connection1, SQL_HANDLE_DBC, "01S02"), OK);

  CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
  CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
  CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

  return OK;
}


ODBC_TEST(sqlerror)
{
  SQLCHAR message[SQL_MAX_MESSAGE_LENGTH + 1];
  SQLCHAR sqlstate[SQL_SQLSTATE_SIZE + 1];
  SQLINTEGER error;
  SQLSMALLINT len;

  ERR_SIMPLE_STMT(Stmt, "SELECT * FROM tabledoesnotexist");
  /*
  CHECK_STMT_RC(Stmt, SQLError(Env, Connection, Stmt, sqlstate, &error,
                          message, sizeof(message), &len));
                          */
  /* Message has been consumed. */
  /*
  FAIL_IF(SQLError(Env, Connection, Stmt, sqlstate, &error,
                              message, sizeof(message), &len) != SQL_NO_DATA_FOUND, "eof expected");
                              */
  /* But should still be available using SQLGetDiagRec. */
  CHECK_STMT_RC(Stmt, SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, sqlstate, &error,
                               message, sizeof(message), &len));

  return OK;
}


/**
 Bug #27158: MyODBC 3.51/ ADO cmd option adCmdUnknown won't work for tables - regression
*/
ODBC_TEST(t_bug27158)
{
  ERR_SIMPLE_STMT(Stmt, "CALL test.does_not_exist");
  return check_sqlstate(Stmt, "42000");
}


ODBC_TEST(t_bug13542600)
{
  SQLINTEGER i;

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "select 1 as i , null as j ");
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_LONG, &i, 0, NULL));

  FAIL_IF(SQLFetch(Stmt)!= SQL_ERROR, "error expected");

  return check_sqlstate(Stmt, "22002");
}


ODBC_TEST(t_bug14285620)
{
  SQLSMALLINT cblen, data_type, dec_digits, nullable;
  SQLUINTEGER info;
  SQLULEN col_size; 
  SQLINTEGER timeout= 20, cbilen;
  SQLCHAR szData[255]={0};

  /* Numeric attribute */
  FAIL_IF(SQLGetConnectAttr(Connection, SQL_ATTR_LOGIN_TIMEOUT, NULL, 0, NULL) != SQL_SUCCESS, "success expected");
  FAIL_IF(SQLGetConnectAttr(Connection, SQL_ATTR_LOGIN_TIMEOUT, &timeout, 0, NULL) != SQL_SUCCESS, "success expected");
  is_num(timeout, 0);
  
  /* Character attribute */
  /* 
    In this particular case MSSQL always returns SQL_SUCCESS_WITH_INFO
    apparently because the driver manager always provides the buffer even
    when the client application passes NULL
  */
#ifdef NOTSKIO
  /* 
    This check is only relevant to Windows Driver Manager 
  */
  FAIL_IF(SQLGetConnectAttr(Connection, SQL_ATTR_CURRENT_CATALOG, NULL, 0, NULL) != SQL_SUCCESS_WITH_INFO, "swi expected");

  FAIL_IF(SQLGetConnectAttr(Connection, SQL_ATTR_CURRENT_CATALOG, szData, 0, NULL) != SQL_SUCCESS_WITH_INFO, "swi expected");
#endif
  /*
  MSDN Says about the last parameter &cblen for SQLGetInfo,
  functions:
  
     If InfoValuePtr is NULL, StringLengthPtr will still return the 
     total number of bytes (excluding the null-termination character 
     for character data) available to return in the buffer pointed 
     to by InfoValuePtr.

     http://msdn.microsoft.com/en-us/library/ms710297%28v=vs.85%29
  */

  FAIL_IF( SQLGetInfo(Connection, SQL_AGGREGATE_FUNCTIONS, NULL, 0, NULL)!= SQL_SUCCESS, "success expected");
  FAIL_IF( SQLGetInfo(Connection, SQL_AGGREGATE_FUNCTIONS, &info, 0, &cblen)!= SQL_SUCCESS, "success expected");
  is_num(cblen, 4);

  is_num(info, (SQL_AF_ALL | SQL_AF_AVG | SQL_AF_COUNT | SQL_AF_DISTINCT | 
             SQL_AF_MAX | SQL_AF_MIN | SQL_AF_SUM));

  /* Get database name for further checks */
  FAIL_IF( SQLGetInfo(Connection, SQL_DATABASE_NAME, szData, sizeof(szData), NULL)!= SQL_SUCCESS, "success expected");
  FAIL_IF( SQLGetInfo(Connection, SQL_DATABASE_NAME, NULL, 0, &cblen)!= SQL_SUCCESS, "success expected");

#ifdef _WIN32  
  /* Windows uses unicode driver by default */
  is_num(cblen, strlen(szData)*sizeof(SQLWCHAR));
#else
  is_num(cblen, strlen(szData));
#endif

  is_num( SQLGetInfo(Connection, SQL_DATABASE_NAME, szData, 0, NULL), SQL_SUCCESS);

  /* Get the native string for further checks */
  FAIL_IF( SQLNativeSql(Connection, "SELECT 10", SQL_NTS, szData, sizeof(szData), NULL) !=SQL_SUCCESS, "success expected");
  FAIL_IF( SQLNativeSql(Connection, "SELECT 10", SQL_NTS, NULL, 0, &cbilen)!= SQL_SUCCESS, "success expected");
  
  /* Do like MSSQL, which does calculate as char_count*sizeof(SQLWCHAR) */
  is_num(cbilen, strlen(szData));

  FAIL_IF( SQLNativeSql(Connection, "SELECT 10", SQL_NTS, szData, 0, NULL)!= SQL_SUCCESS_WITH_INFO, "swi expected");

  /* Get the cursor name for further checks */
  FAIL_IF(SQLGetCursorName(Stmt, szData, sizeof(szData), NULL) != SQL_SUCCESS, "success expected");
  FAIL_IF(!SQL_SUCCEEDED(SQLGetCursorName(Stmt, NULL, 0, &cblen)), "success expected");
  
  /* Do like MSSQL, which does calculate as char_count*sizeof(SQLWCHAR) */
  is_num(cblen, strlen(szData));

  FAIL_IF(SQLGetCursorName(Stmt, szData, 0, NULL) != SQL_SUCCESS_WITH_INFO, "swi expected");

  FAIL_IF(SQLExecDirect(Stmt, "ERROR SQL QUERY", SQL_NTS) != SQL_ERROR, "Error expected");

  FAIL_IF(SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 1, SQL_DIAG_SQLSTATE, NULL, 0, NULL) != SQL_SUCCESS, "success expected");

  {
    SQLCHAR sqlstate[30], message[255];
    SQLINTEGER native_error= 0;
    SQLSMALLINT text_len= 0;
    /* try with the NULL pointer for Message */
    FAIL_IF(SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, sqlstate,
                                    &native_error, NULL, 0, &cblen) != SQL_SUCCESS, "success expected");
    /* try with the non-NULL pointer for Message */
    FAIL_IF(SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, sqlstate,
                                    &native_error, message, 0, NULL) != SQL_SUCCESS, "success expected");
  }

  SQLExecDirect(Stmt, "drop table bug14285620", SQL_NTS);

  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "CREATE TABLE bug14285620 (id INT)", SQL_NTS));
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "INSERT INTO bug14285620 (id) VALUES (1)", SQL_NTS));
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "SELECT * FROM bug14285620", SQL_NTS));

  FAIL_IF(SQLDescribeCol(Stmt, 1, NULL, 0, NULL, &data_type, &col_size, &dec_digits, &nullable) != SQL_SUCCESS, "Success expected");
  FAIL_IF(SQLDescribeCol(Stmt, 1, szData, 0, &cblen, &data_type, &col_size, &dec_digits, &nullable) != SQL_SUCCESS_WITH_INFO, "swi expected");

  FAIL_IF(SQLColAttribute(Stmt,1, SQL_DESC_TYPE, NULL, 0, NULL, NULL) != SQL_SUCCESS, "Success expected");
  FAIL_IF(SQLColAttribute(Stmt,1, SQL_DESC_TYPE, &data_type, 0, NULL, NULL)!= SQL_SUCCESS, "Success expected");

  FAIL_IF(SQLColAttribute(Stmt,1, SQL_DESC_TYPE_NAME, NULL, 0, NULL, NULL) != SQL_SUCCESS, "Success expected");
  FAIL_IF(SQLColAttribute(Stmt,1, SQL_DESC_TYPE_NAME, szData, 0, NULL, NULL) != SQL_SUCCESS_WITH_INFO, "swi expected");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  return OK;
}

/*
  Bug49466: SQLMoreResults does not set statement errors correctly
  Wrong error message returned from SQLMoreResults
*/
ODBC_TEST(t_bug49466)
{
  SQLHDBC  Connection1;
  SQLHSTMT Stmt1;
  SQLCHAR conn[512];
  SQLCHAR message[SQL_MAX_MESSAGE_LENGTH + 1];
  SQLCHAR sqlstate[SQL_SQLSTATE_SIZE + 1];
  SQLINTEGER error;
  SQLSMALLINT len;

  sprintf((char *)conn, "DSN=%s;UID=%s;PWD=%s;OPTION=67108864", 
                        my_dsn, my_uid, my_pwd);

  CHECK_ENV_RC(Env, SQLAllocHandle(SQL_HANDLE_DBC, Env, &Connection1));

  CHECK_DBC_RC(Connection1, SQLDriverConnect(Connection1, NULL, conn, sizeof(conn), NULL,
                                 0, NULL,
                                 SQL_DRIVER_NOPROMPT));
  CHECK_DBC_RC(Connection1, SQLAllocStmt(Connection1, &Stmt1));

  ERR_SIMPLE_STMT(Stmt1, "CALL t_bug49466proc()");
   
  
  CHECK_STMT_RC(Stmt, SQLGetDiagRec(SQL_HANDLE_STMT, Stmt1, 1, sqlstate, &error,
                               message, sizeof(message), &len));
  is_num(error, 1305);
  
  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1,SQL_DROP));
  CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
  CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));

  return OK;
}





MA_ODBC_TESTS my_tests[]=
{
  {t_odbc3_error, "t_odbc3_error"},
  {t_odbc2_error, "t_odbc2_error"},
  {t_diagrec, "t_diagrec"},
  {t_warning, "t_warning"},
  {t_bug3456, "t_bug3456"},
  {t_bug16224, "t_bug16224"},
  {bind_invalidcol, "bind_invalidcol"},
  {bind_notenoughparam1, "bind_notenoughparam1"},
  {bind_notenoughparam2, "bind_notenoughparam2" },
  {getdata_need_nullind, "getdata_need_nullind"},
  {t_handle_err, "t_handle_err"},
  {sqlerror, "sqlerror"},
  {t_bug27158, "t_bug27158"},
  {t_bug13542600, "t_bug13542600"},
  {t_bug14285620, "t_bug14285620"},
  {t_bug49466, "t_bug49466"},
  {NULL, NULL}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}
