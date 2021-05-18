/* 
 * File:   csqliteapi.h
 * Author: wyy
 *
 * Created on 2009Ôø?Ôø?8Ôø? ‰∏äÂçà9:30
 */

#ifndef _CSQLITEAPI_H
#define	_CSQLITEAPI_H



using namespace std;

#include "sqlite3.h"

#define ERROR_SQLITE_INVALIDATE_DB  -10003
#define ERROR_SQLITE_INTERNAL       -10004
#define ERROR_SQLITE_DB_NOT_OPEN    -10005
#define ERROR_SQLITE_INVALIDATE_SQL -10006

class CSqliteApi {
private:
    bool        m_bIsOpen;
    bool        m_bTransaction;
    string      m_szDatabase;
    string      m_szLastSql;
    string*     m_arrayCols;
    int         m_nFields;
    int         m_nRecords;
    sqlite3*    m_pSqlite3;
    char**      m_queryResult;
public:
    CSqliteApi();
    CSqliteApi(const CSqliteApi& orig);
    virtual ~CSqliteApi();
private:
    void    m_Clear();
public:
    int     m_Open(string szTableName);
    void    m_Close();
    bool    m_IsOpen();
    int     m_Query(string& szSQL);
    int     m_Exec(string& szSQL);
    int     m_TransactionBegin();
    int     m_TransactionAdd(string szSQL);
    int     m_TransactionOver();
    string* m_GetColumns(string szSQL);
    char**  m_GetRecord(int nRecord);				//nRecord is 1 based index
    char*   m_GetField(char** sRecord,int nField);	//nField is 0 based index
    int     m_GetRecordsCount();
    int     m_GetFieldsCount();
	const char*	m_GetLassError();
};

#endif	/* _CSQLITEAPI_H */

