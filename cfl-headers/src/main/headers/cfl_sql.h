/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef CFL_SQL_H_

#define CFL_SQL_H_

#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CFL_SQL_TO_STRING(sql, str) ((CFL_SQLP)sql)->to_string((CFL_SQLP)sql, str)
#define CFL_SQL_FREE(sql) ((CFL_SQLP)sql)->free_sql((CFL_SQLP)sql)

typedef void (* CFL_SQL_TO_STRING_FUN )(CFL_SQLP element, CFL_STRP str);
typedef void (* CFL_SQL_FREE_FUN )(CFL_SQLP item);

typedef CFL_SQL_QUERYP (* CFL_SQL_NEW_QUERY_FUN )(void);
typedef CFL_SQL_INSERTP (* CFL_SQL_NEW_INSERT_FUN )(void);
typedef CFL_SQL_UPDATEP (* CFL_SQL_NEW_UPDATE_FUN )(void);
typedef CFL_SQL_DELETEP (* CFL_SQL_NEW_DELETE_FUN )(void);
typedef CFL_SQL_BLOCKP (* CFL_SQL_NEW_BLOCK_FUN )(void);

typedef CFL_SQL_QUERYP (* CFL_SQL_QUERY_ITEMS_FUN )(CFL_SQL_QUERYP query, CFL_SQLP first, ...);
typedef CFL_SQL_QUERYP (* CFL_SQL_QUERY_ITEM_FUN )(CFL_SQL_QUERYP query, CFL_SQLP item);
typedef CFL_SQL_QUERYP (* CFL_SQL_QUERY_HINT_FUN )(CFL_SQL_QUERYP query, CFL_STRP value);
typedef CFL_SQL_QUERYP (* CFL_SQL_QUERY_UPDATE_FUN )(CFL_SQL_QUERYP query, CFL_UINT16 wait);

typedef CFL_SQL_INSERTP (* CFL_SQL_INS_ITEM_FUN )(CFL_SQL_INSERTP ins, CFL_SQLP table);
typedef CFL_SQL_INSERTP (* CFL_SQL_INS_ITEMS_FUN )(CFL_SQL_INSERTP ins, CFL_SQLP first, ...);
typedef CFL_SQL_INSERTP (* CFL_SQL_INS_ITEM2_FUN )(CFL_SQL_INSERTP ins, CFL_SQLP item1, CFL_SQLP item2);

typedef CFL_SQL_UPDATEP (* CFL_SQL_UPD_ITEM_FUN )(CFL_SQL_UPDATEP upd, CFL_SQLP item);
typedef CFL_SQL_UPDATEP (* CFL_SQL_UPD_ITEM2_FUN )(CFL_SQL_UPDATEP upd, CFL_SQLP item1, CFL_SQLP item2);

typedef CFL_SQL_DELETEP (* CFL_SQL_DEL_ITEM_FUN )(CFL_SQL_DELETEP del, CFL_SQLP item);
typedef CFL_SQL_DELETEP (* CFL_SQL_DEL_ITEM2_FUN )(CFL_SQL_DELETEP del, CFL_SQLP item1, CFL_SQLP item2);

typedef CFL_SQL_DELETEP (* CFL_SQL_DEL_ITEM_FUN )(CFL_SQL_DELETEP del, CFL_SQLP item);
typedef CFL_SQL_DELETEP (* CFL_SQL_DEL_ITEM2_FUN )(CFL_SQL_DELETEP del, CFL_SQLP item1, CFL_SQLP item2);

typedef CFL_SQL_BLOCKP (* CFL_SQL_BLOCK_ITEM_FUN )(CFL_SQL_BLOCKP block, CFL_SQLP item);
typedef CFL_SQL_BLOCKP (* CFL_SQL_BLOCK_PRAGMA_FUN )(CFL_SQL_BLOCKP block, CFL_STRP item);
typedef CFL_SQL_BLOCKP (* CFL_SQL_BLOCK_CHAR_PRAGMA_FUN )(CFL_SQL_BLOCKP block, char *item);
typedef CFL_SQL_BLOCKP (* CFL_SQL_BLOCK_VAR_FUN )(CFL_SQL_BLOCKP block, CFL_STRP varName, CFL_SQLP varType, CFL_SQLP varValue);
typedef CFL_SQL_BLOCKP (* CFL_SQL_BLOCK_CHAR_VAR_FUN )(CFL_SQL_BLOCKP block, char *varName, CFL_SQLP varType, CFL_SQLP varValue);

typedef CFL_SQLP (* CFL_SQL_NO_ARG_FUN )(void);
typedef CFL_SQLP (* CFL_SQL_FORMAT_FUN )(const char *fmt, ...);
typedef CFL_SQLP (* CFL_SQL_CHAR_FUN )(char *value);
typedef CFL_SQLP (* CFL_SQL_MULTI_CHAR_FUN )(char *value, ...);
typedef CFL_SQLP (* CFL_SQL_STR_FUN )(CFL_STRP value);
typedef CFL_SQLP (* CFL_SQL_MULTI_STR_FUN )(CFL_STRP name, ...);
typedef CFL_SQLP (* CFL_SQL_SQL_FUN )(CFL_SQLP expr);
typedef CFL_SQLP (* CFL_SQL_SQL2_FUN )(CFL_SQLP left, CFL_SQLP right);
typedef CFL_SQLP (* CFL_SQL_SQL_STR_FUN )(CFL_SQLP expr, CFL_STRP alias);
typedef CFL_SQLP (* CFL_SQL_CALL_FUN )(CFL_STRP funName, ...);
typedef CFL_SQLP (* CFL_SQL_CALL_CHAR_FUN )(char *funName, ...);

struct _CFL_SQL {
   CFL_SQL_TO_STRING_FUN to_string;
   CFL_SQL_FREE_FUN      free_sql;
};

struct _CFL_SQL_QUERY {
   CFL_SQL                  sql;
   CFL_STRP                 hintValue;
   CFL_LISTP                columns;
   CFL_LISTP                datasets;
   CFL_SQLP                 condition;
   CFL_LISTP                orders;
   CFL_SQL_QUERY_HINT_FUN   hint;
   CFL_SQL_QUERY_ITEMS_FUN  select;
   CFL_SQL_QUERY_ITEMS_FUN  from;
   CFL_SQL_QUERY_ITEM_FUN   where;
   CFL_SQL_QUERY_ITEMS_FUN  orderBy;
   CFL_SQL_QUERY_UPDATE_FUN forUpdate;
   CFL_BOOL                 lock;
   CFL_UINT16               wait;
};

struct _CFL_SQL_INSERT {
   CFL_SQL               sql;
   CFL_SQLP              tableName;
   CFL_LISTP             columnsNames;
   CFL_LISTP             columnsValues;
   CFL_LISTP             returningColumns;
   CFL_LISTP             returningParams;
   CFL_SQL_INS_ITEM_FUN  into;
   CFL_SQL_INS_ITEMS_FUN columns;
   CFL_SQL_INS_ITEMS_FUN values;
   CFL_SQL_INS_ITEM2_FUN returning;
};

struct _CFL_SQL_UPDATE {
   CFL_SQL               sql;
   CFL_SQLP              tableName;
   CFL_LISTP             columnsNames;
   CFL_LISTP             columnsValues;
   CFL_SQLP              condition;
   CFL_LISTP             returningColumns;
   CFL_LISTP             returningParams;
   CFL_SQL_UPD_ITEM_FUN  table;
   CFL_SQL_UPD_ITEM2_FUN set;
   CFL_SQL_UPD_ITEM_FUN  where;
   CFL_SQL_UPD_ITEM2_FUN returning;
};

struct _CFL_SQL_DELETE {
   CFL_SQL               sql;
   CFL_SQLP              tableName;
   CFL_SQLP              condition;
   CFL_LISTP             returningColumns;
   CFL_LISTP             returningParams;
   CFL_SQL_DEL_ITEM_FUN  from;
   CFL_SQL_DEL_ITEM_FUN  where;
   CFL_SQL_DEL_ITEM2_FUN returning;
};

struct _CFL_SQL_BLOCK {
   CFL_SQL                  sql;
   CFL_LISTP                pragmas;
   CFL_LISTP                vars;
   CFL_LISTP                statements;
   CFL_SQL_BLOCK_VAR_FUN    var;
   CFL_SQL_BLOCK_PRAGMA_FUN pragma;
   CFL_SQL_BLOCK_ITEM_FUN   statement;
};

struct _CFL_SQL_CUSTOM {
   CFL_SQL  sql;
   CFL_STRP value;
};

struct _CFL_SQL_FUN {
   CFL_SQL   sql;
   CFL_STRP  name;
   CFL_LISTP args;
};

struct _CFL_SQL_DOUBLE_OP {
   CFL_SQL  sql;
   CFL_SQLP left;
   CFL_STRP op;
   CFL_SQLP right;
};

struct _CFL_SQL_SINGLE_OP {
   CFL_SQL  sql;
   CFL_STRP op;
   CFL_SQLP expr;
};

struct _CFL_SQL_WRAP {
   CFL_SQL  sql;
   CFL_SQLP expr;
};

struct _CFL_SQL_BUILDER {
   CFL_SQL_NEW_QUERY_FUN  query;
   CFL_SQL_NEW_INSERT_FUN insert;
   CFL_SQL_NEW_UPDATE_FUN update;
   CFL_SQL_NEW_DELETE_FUN delete;
   CFL_SQL_STR_FUN        id;
   CFL_SQL_MULTI_STR_FUN  quali_id;
   CFL_SQL_STR_FUN        param;
   CFL_SQL_CHAR_FUN       c_id;
   CFL_SQL_MULTI_CHAR_FUN c_quali_id;
   CFL_SQL_CHAR_FUN       c_param;
   CFL_SQL_SQL_STR_FUN    as;
   CFL_SQL_SQL2_FUN       equal;
   CFL_SQL_SQL2_FUN       diff;
   CFL_SQL_SQL2_FUN       in;
   CFL_SQL_SQL2_FUN       notIn;
   CFL_SQL_SQL2_FUN       exists;
   CFL_SQL_SQL2_FUN       notExists;
   CFL_SQL_SQL2_FUN       like;
   CFL_SQL_SQL2_FUN       greater;
   CFL_SQL_SQL2_FUN       less;
   CFL_SQL_SQL2_FUN       greaterEqual;
   CFL_SQL_SQL2_FUN       lessEqual;
   CFL_SQL_SQL_FUN        isNull;
   CFL_SQL_SQL_FUN        isNotNull;
   CFL_SQL_SQL2_FUN       and;
   CFL_SQL_SQL2_FUN       or;
   CFL_SQL_SQL_FUN        parentheses;
   CFL_SQL_SQL_FUN        desc;
   CFL_SQL_SQL_FUN        asc;
   CFL_SQL_CALL_FUN       fun;
   CFL_SQL_CALL_CHAR_FUN  c_fun;
   CFL_SQL_STR_FUN        custom;
   CFL_SQL_CHAR_FUN       c_custom;
   CFL_SQL_FORMAT_FUN     format;
   CFL_SQL_SQL2_FUN       plus;
   CFL_SQL_SQL2_FUN       minus;
   CFL_SQL_SQL2_FUN       div;
   CFL_SQL_SQL2_FUN       mult;
   CFL_SQL_NO_ARG_FUN     commit;
   CFL_SQL_NO_ARG_FUN     rollback;
   CFL_SQL_NO_ARG_FUN     null;
   CFL_SQL_NEW_BLOCK_FUN  block;
};

extern void cfl_sql_initBuilder(CFL_SQL_BUILDERP builder);

#ifdef __cplusplus
}
#endif

#endif
