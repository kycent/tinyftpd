/*
 * ftp_db.h
 *
 *  Created on: 2017-3-15
 *      Author: gaokai
 */

#ifndef FTP_DB_H_
#define FTP_DB_H_


#define DB_FIELD_MAX_LEN (128)
#define DB_FIELD_NUM_MAX (64)
#define DB_FIELD_NAME_LEN (64)
#define DB_TABLE_NAME_LEN (64)

typedef enum
{
	DB_FIELD_TYPE_INT = 0,
	DB_FIELD_TYPE_SHORT = 1,
	DB_FIELD_TYPE_TEXT = 2,
}DB_FIELD_TYPE;


typedef struct db_feild
{
	DB_FIELD_TYPE field_type;
	int isKey;
	char name[DB_FIELD_NAME_LEN];
	char data[DB_FIELD_MAX_LEN];
}db_field_t;


typedef struct db_record
{
	db_field_t fields[DB_FIELD_MAX_LEN];
	int field_num;
}db_record_t;


typedef struct db_table
{
	db_field_t field[DB_FIELD_NUM_MAX];
	char name[DB_TABLE_NAME_LEN];
	int field_num;
}db_table_t;


typedef enum
{
	DB_OP_TYPE_INSERT,
	DB_OP_TYPE_UPDATE,
	DB_OP_TYPE_DELETE,
	DB_OP_TYPE_SELECT_BY_KEY,
	DB_OP_TYPE_SELECT_ALL,
}DB_OP_TYPE;

typedef enum
{
	DB_SCAN_RETCODE_CONTINUE,
	DB_SCAN_RETCODE_BREAK,
}DB_SCAN_RETCODE;

typedef enum
{
	DB_OK = 0,
	DB_ERRNO_NULL_POINTER = 1,
	DB_ERRNO_NOMEM = 2,
	DB_ERRNO_NOT_FOUND = 3,
	DB_ERRNO_EXISTS = 4,
	DB_ERRNO_CANNOT_OPEN = 5,
	DB_ERRNO_INVALID_PARA = 6,
}DB_ERRNO_E;


typedef struct
{
	DB_OP_TYPE op_type;
	db_record_t condition;
}db_statement_t;

typedef DB_SCAN_RETCODE (* db_scan_func)(db_record_t * record, void * user_data);


#define DB_FIELD_GET_VALUE(field)\
	((field->field_type == DB_FIELD_TYPE_INT) ? (*(int*)field->data) : \
	((field->field_type == DB_FIELD_TYPE_SHORT) ? (*(short *)field->data) : field->data))


#define DB_FIELD_GET_VALUE_STR(field)\
	((field->field_type == DB_FIELD_TYPE_TEXT) ? field->data : atoi(DB_FIELD_GET_VALUE(field)))


/* API Interface begin */
int db_table_add_field(db_table_t * table ,db_field_t * field);

int db_create_table(db_table_t * table);

int db_destory_table(db_table_t * table);

int db_add_or_update_record(db_table_t * table, db_record_t * record);

int db_delete_record(db_table_t * table, db_record_t * record);

int db_find_record_by_key(db_table_t * table, db_record_t * record);

int db_scan(db_table_t * table, db_scan_func scan_cbk, void * user_data);
/* API interface end */

/* Internal functions begin */
int _db_compile_statement_to_sql(db_table_t * table ,db_statement_t * stmt, char * sql);

int _db_engine_sqlite_exec_callback(void * arg, int nr, char ** values, char ** errmsg);

int _db_decode_result_to_record(char * result, db_record_t * record);

int _db_engine_exec_sql(char * sql);
/* Internal functions end */

#endif /* FTP_DB_H_ */
