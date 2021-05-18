/* 
 * File:   csqliteapi.cc
 * Author: wyy
 * 
 * Created on 2009�???8?? �??9:30
 */

#include "stdafx.h"
#include <string>
#include "csqliteapi.h"

CSqliteApi::CSqliteApi() {
    m_arrayCols = NULL;
    m_pSqlite3 = NULL;
    m_queryResult = NULL;
    m_bIsOpen = false;
    m_bTransaction = false;
    m_nFields = 0;
    m_nRecords = 0;
    this->m_Clear();
}

CSqliteApi::CSqliteApi(const CSqliteApi& orig) {
    m_arrayCols = NULL;
    m_queryResult = NULL;
    m_pSqlite3 = NULL;
    m_bIsOpen = false;
    m_nFields = 0;
    m_nRecords = 0;
}

CSqliteApi::~CSqliteApi() {
    this->m_Clear();
}

void CSqliteApi::m_Clear() {
    if( m_arrayCols ) delete [] m_arrayCols;
    sqlite3_free_table(m_queryResult);
    m_arrayCols = NULL;
    m_queryResult = NULL;
    m_pSqlite3 = NULL;
    m_bIsOpen = false;
    m_nFields = 0;
    m_nRecords = 0;
    m_szDatabase.clear();
    m_szLastSql.clear();
}

int CSqliteApi::m_Open(string szDatabase) {
    m_bIsOpen = false;
    if( 0 == szDatabase.size() ) return ERROR_SQLITE_INVALIDATE_DB;
    this->m_Close();
    this->m_szDatabase = szDatabase;
//    int error = sqlite3_open_v2(szDatabase.data(),&this->m_pSqlite3,SQLITE_OPEN_READWRITE,"");
    int error = sqlite3_open(szDatabase.data(),&this->m_pSqlite3);
    if( SQLITE_OK != error ) {
		printf("\nError on open database %s,code = %d",szDatabase.data(),error );
        return -error;
    }
    if( this->m_pSqlite3 ) {
        this->m_bIsOpen = true;
        return SQLITE_OK;
    }
    return ERROR_SQLITE_INTERNAL;
}

void CSqliteApi::m_Close() {
    if( this->m_bIsOpen && this->m_pSqlite3 ) {
        sqlite3_close( this->m_pSqlite3 );
    }
    return this->m_Clear();
}

bool CSqliteApi::m_IsOpen() {
    return this->m_bIsOpen;
}

string* CSqliteApi::m_GetColumns(string szSQL) {
    if( this->m_bIsOpen && this->m_pSqlite3 &&
        this->m_queryResult && this->m_nRecords >= 0 ) {
        sqlite3_stmt*   stmt = NULL;
        const char*     zTail = NULL;
        if( SQLITE_OK == sqlite3_prepare(this->m_pSqlite3,szSQL.data(),-1,&stmt,&zTail) ) {
            if (this->m_arrayCols) delete [] this->m_arrayCols;
            this->m_nFields = sqlite3_column_count(stmt);
            this->m_arrayCols = new string[this->m_nFields];
            for( int n = 0; n < this->m_nFields; n ++ ) {
                this->m_arrayCols[n] = sqlite3_column_name(stmt, n);
            }
            if (stmt) sqlite3_finalize(stmt);
            return this->m_arrayCols;
        }
        else return NULL;
    }
    return NULL;
}

char** CSqliteApi::m_GetRecord(int nRecord) {
    if( this->m_bIsOpen && this->m_pSqlite3 && this->m_queryResult &&
        this->m_nRecords >= nRecord && nRecord >= 1 ) {
        return &this->m_queryResult[nRecord*this->m_nFields];
    }
    return NULL;
}

char* CSqliteApi::m_GetField(char** sRecord,int nField) {
    if( this->m_bIsOpen && this->m_pSqlite3 && sRecord &&
        this->m_nFields > nField && nField >= 0 ) {
        return sRecord[nField];
    }
    return NULL;
}

int CSqliteApi::m_GetRecordsCount() {
    return this->m_nRecords;
}

int CSqliteApi::m_GetFieldsCount() {
    return this->m_nFields;
}

int CSqliteApi::m_TransactionBegin() {
    if( NULL == this->m_pSqlite3 || false == this->m_bIsOpen ) return ERROR_SQLITE_DB_NOT_OPEN;
    if (SQLITE_OK == sqlite3_exec(this->m_pSqlite3, "begin;", 0, 0, 0)) m_bTransaction = true;
    else m_bTransaction = false;
    return m_bTransaction;
}

int CSqliteApi::m_TransactionAdd(string szSQL) {
    if( NULL == this->m_pSqlite3 || false == this->m_bIsOpen ) {
        m_bTransaction = ERROR_SQLITE_DB_NOT_OPEN;
        return m_bTransaction;
    }
    if( 0 == szSQL.size() ) {
        m_bTransaction = ERROR_SQLITE_INVALIDATE_SQL;
        return m_bTransaction;
    }
    if( string::npos != szSQL.find("select") || string::npos != szSQL.find("SELECT")) {
        return SQLITE_OK;
    }
    if( m_bTransaction ) {
        if (SQLITE_OK == sqlite3_exec(this->m_pSqlite3, szSQL.data(), 0, 0, 0))
            m_bTransaction = true;
        else m_bTransaction = false;
    }
    return m_bTransaction;
}

int CSqliteApi::m_TransactionOver() {
    if( NULL == this->m_pSqlite3 || false == this->m_bIsOpen ) return ERROR_SQLITE_DB_NOT_OPEN;
    if (m_bTransaction)
        return sqlite3_exec(this->m_pSqlite3, "commit;", 0, 0, 0);  // 提交事务
    else
        return sqlite3_exec(this->m_pSqlite3, "rollback;", 0, 0, 0);  // 回滚事务
}

int CSqliteApi::m_Exec(string& szSQL) {
    char*   eMsg = NULL;
    if( NULL == this->m_pSqlite3 || false == this->m_bIsOpen ) return ERROR_SQLITE_DB_NOT_OPEN;
    if( 0 == szSQL.size() ) return ERROR_SQLITE_INVALIDATE_SQL;
    this->m_szLastSql.clear();
    this->m_szLastSql = szSQL;
    int     rc = sqlite3_exec(this->m_pSqlite3, szSQL.data(), 0, 0, &eMsg);
    if( eMsg ) {
        printf(eMsg);
        sqlite3_free(eMsg);
    }
    return rc;
}

int CSqliteApi::m_Query(string& szSQL) {
    char*   eMsg = NULL;
    if( NULL == this->m_pSqlite3 || false == this->m_bIsOpen ) return ERROR_SQLITE_DB_NOT_OPEN;
    if( 0 == szSQL.size() ) return ERROR_SQLITE_INVALIDATE_SQL;
    this->m_szLastSql.clear();
    this->m_szLastSql = szSQL;
    sqlite3_free_table(this->m_queryResult);
    int rc = sqlite3_get_table(   this->m_pSqlite3,this->m_szLastSql.data(),&this->m_queryResult,
                                &this->m_nRecords,&this->m_nFields,&eMsg);
    if( eMsg ) {
        printf(eMsg);
        sqlite3_free(eMsg);
    }
    return rc;
}

const char* CSqliteApi::m_GetLassError() {
	if (NULL == this->m_pSqlite3 || false == this->m_bIsOpen) return (const char*)"Not Open";
	return sqlite3_errmsg(this->m_pSqlite3);
}
