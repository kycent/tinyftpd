/*
 * ftp_db.c
 *
 *  Created on: 2017-3-15
 *      Author: gaokai
 */

#include <sqlite3.h>
#include <stdlib.h>


int _db_compile_statement_to_sql(db_table_t * table ,db_statement_t * stmt, char * sql)
{
	char buff[1024] = {0};
	int loop = 0;

	if (NULL == stmt || NULL == sql || NULL == table){
		return DB_ERRNO_NULL_POINTER;
	}

	if (stmt->op_type > DB_OP_TYPE_SELECT_ALL){
		return DB_ERRNO_INVALID_PARA;
	}

	switch(stmt->op_type){
		case DB_OP_TYPE_INSERT:
		{
			sprintf(buff, "INSERT INTO %s (", table->name);
			for (loop = 0; loop < stmt->condition.field_num; loop++)
			{
				strcat(buff, stmt->condition.fields[loop].name);
				if (loop != stmt->condition.field_num - 1){
					strcat(buff, ",");
				}
			}
			strcat(buff, ") VALUES(");

			for (loop = 0; loop < stmt->condition.field_num; loop++)
			{
				strcat(buff, DB_FIELD_GET_VALUE_STR(&stmt->condition.fields[loop]));
				if (loop != stmt->condition.field_num){
					strcat(buff, ",");
				}
			}
			strcat(buff, ")");

			sprintf(sql, buff);
			break;
		}
		case DB_OP_TYPE_DELETE:
		{
			break;
		}
		case DB_OP_TYPE_SELECT_ALL:
		{
			break;
		}
		case DB_OP_TYPE_SELECT_BY_KEY:
		{
			break;
		}
		case DB_OP_TYPE_UPDATE:
		{
			break;
		}
		default:
		{
			return DB_ERRNO_INVALID_PARA;
		}
	}
	/* firstly, get the operation name, such as select, insert, update, delete */
}


