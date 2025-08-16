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

#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>

#include "cfl_sql.h"
#include "cfl_str.h"
#include "cfl_list.h"
#include "cfl_mem.h"

#define DOUBLE_OP(fun_name, oper) static CFL_SQLP fun_name(CFL_SQLP left, CFL_SQLP right) { \
                                     CFL_SQL_DOUBLE_OPP newOp = (CFL_SQL_DOUBLE_OPP) CFL_MEM_ALLOC(sizeof(CFL_SQL_DOUBLE_OP)); \
                                     newOp->sql.to_string = double_op_to_string; \
                                     newOp->sql.free_sql = double_op_free; \
                                     newOp->left = left; \
                                     newOp->op = cfl_str_newConstLen(oper, sizeof(oper) - 1); \
                                     newOp->right = right; \
                                     return (CFL_SQLP) newOp; \
                                  }

#define POS_OP(fun_name, oper) static CFL_SQLP fun_name(CFL_SQLP expr) { \
                                  CFL_SQL_SINGLE_OPP newOp = (CFL_SQL_SINGLE_OPP) CFL_MEM_ALLOC(sizeof(CFL_SQL_SINGLE_OP)); \
                                  newOp->sql.to_string = pos_op_to_string; \
                                  newOp->sql.free_sql = single_op_free; \
                                  newOp->expr = expr; \
                                  newOp->op = cfl_str_newConstLen(oper, sizeof(oper) - 1); \
                                  return (CFL_SQLP) newOp; \
                               }

#define CONST_CUSTOM_SQL(fun_name, str) static CFL_SQLP fun_name(void) { \
                                           CFL_SQL_CUSTOMP stmt = (CFL_SQL_CUSTOMP) CFL_MEM_ALLOC(sizeof(CFL_SQL_CUSTOM)); \
                                           stmt->sql.to_string = custom_to_string; \
                                           stmt->sql.free_sql = custom_free; \
                                           stmt->value = cfl_str_newConstLen(str, sizeof(str) - 1); \
                                           return (CFL_SQLP) stmt; \
                                        }

#define CUSTOM_SQL(fun_name) static CFL_SQLP fun_name(CFL_STRP str) { \
                                CFL_SQL_CUSTOMP stmt = (CFL_SQL_CUSTOMP) CFL_MEM_ALLOC(sizeof(CFL_SQL_CUSTOM)); \
                                stmt->sql.to_string = custom_to_string; \
                                stmt->sql.free_sql = custom_free; \
                                stmt->value = cfl_str_newStr(str); \
                                return (CFL_SQLP) stmt; \
                             }

#define C_CUSTOM_SQL(fun_name) static CFL_SQLP fun_name(char *str) { \
                                  CFL_SQL_CUSTOMP stmt = (CFL_SQL_CUSTOMP) CFL_MEM_ALLOC(sizeof(CFL_SQL_CUSTOM)); \
                                  stmt->sql.to_string = custom_to_string; \
                                  stmt->sql.free_sql = custom_free; \
                                  stmt->value = cfl_str_newBuffer(str); \
                                  return (CFL_SQLP) stmt; \
                               }

#define CUSTOM_SQL2(fun_name, str_fun) static CFL_SQLP fun_name(CFL_STRP str) { \
                                          CFL_SQL_CUSTOMP stmt = (CFL_SQL_CUSTOMP) CFL_MEM_ALLOC(sizeof(CFL_SQL_CUSTOM)); \
                                          stmt->sql.to_string = str_fun; \
                                          stmt->sql.free_sql = custom_free; \
                                          stmt->value = cfl_str_newStr(str); \
                                          return (CFL_SQLP) stmt; \
                                       }
#define C_CUSTOM_SQL2(fun_name, str_fun) static CFL_SQLP fun_name(char *str) { \
                                            CFL_SQL_CUSTOMP stmt = (CFL_SQL_CUSTOMP) CFL_MEM_ALLOC(sizeof(CFL_SQL_CUSTOM)); \
                                            stmt->sql.to_string = str_fun; \
                                            stmt->sql.free_sql = custom_free; \
                                            stmt->value = cfl_str_newBuffer(str); \
                                            return (CFL_SQLP) stmt; \
                                         }

static CFL_LISTP list_add_items(CFL_LISTP list, CFL_SQLP first, va_list args) {
   CFL_SQLP item;
   
   if (list == NULL) {
      list = cfl_list_new(8);
   }
   if (first != NULL) {
      cfl_list_add(list, first);
   }
   item = va_arg(args, CFL_SQLP);
   while ( item != NULL ) {
      cfl_list_add(list, item);
      item = va_arg(args, CFL_SQLP);
   }
   return list;
}

static void list_free(CFL_LISTP list) {
   CFL_SQLP item;
   CFL_UINT32 i;
   CFL_UINT32 len;
   
   len = cfl_list_length(list);
   for (i = 0; i < len; i++) {
      item = (CFL_SQLP) cfl_list_get(list, i);
      item->free_sql(item);
   }
   cfl_list_free(list);
}

static void list_to_string(CFL_LISTP list, CFL_STRP str, char *sep) {
   CFL_SQLP item;
   CFL_UINT32 i;
   CFL_UINT32 len;
   CFL_UINT32 sepLen = (CFL_UINT32) (sep != NULL ? strlen(sep) : 0);
   CFL_BOOL first = CFL_TRUE;
   len = cfl_list_length(list);
   for (i = 0; i < len; i++) {
      item = (CFL_SQLP) cfl_list_get(list, i);
      if (sepLen > 0) {
         if (first) {
            first = CFL_FALSE;
         } else {
            cfl_str_appendLen(str, sep, sepLen);
         }
      }
      item->to_string(item, str);
   }
}

static void double_list_to_string(CFL_LISTP list1, CFL_LISTP list2, CFL_STRP str, char *sep1, char *sep2) {
   CFL_SQLP item1;
   CFL_SQLP item2;
   CFL_UINT32 i;
   CFL_UINT32 len;
   CFL_UINT32 sep1Len = (CFL_UINT32) (sep1 != NULL ? strlen(sep1) : 0);
   CFL_UINT32 sep2Len = (CFL_UINT32) (sep2 != NULL ? strlen(sep2) : 0);
   CFL_BOOL first = CFL_TRUE;

   len = cfl_list_length(list1);
   for (i = 0; i < len; i++) {
      item1 = (CFL_SQLP) cfl_list_get(list1, i);
      item2 = (CFL_SQLP) cfl_list_get(list2, i);
      if (item1 != NULL && item2 != NULL) {
         if (sep2Len > 0) {
            if (first) {
               first = CFL_FALSE;
            } else {
               cfl_str_appendLen(str, sep2, sep2Len);
            }
         }
         item1->to_string(item1, str);
         if (sep1Len > 0) {
            cfl_str_appendLen(str, sep1, sep1Len);
         }
         item2->to_string(item2, str);
      }
   }
}

/***************************
 * QUERY
 ***************************/
static void query_to_string(CFL_SQLP sql, CFL_STRP str) {
   CFL_SQL_QUERYP query = (CFL_SQL_QUERYP) sql;
   CFL_STR_APPEND_CONST(str, "select ");
   if (query->hintValue != NULL) {
      CFL_STR_APPEND_CONST(str, "/*+ ");
      cfl_str_appendStr(str, query->hintValue);
      CFL_STR_APPEND_CONST(str, " */ ");
   }
   if (query->columns != NULL) {
      list_to_string(query->columns, str, ",");
   }
   CFL_STR_APPEND_CONST(str, " from ");
   if (query->datasets != NULL) {
      list_to_string(query->datasets, str, ",");
   }
   if (query->condition != NULL) {
      CFL_STR_APPEND_CONST(str, " where ");
      query->condition->to_string(query->condition, str);
   }
   if (query->orders != NULL) {
      CFL_STR_APPEND_CONST(str, " order by ");
      list_to_string(query->orders, str, ", ");
   }
   if (query->lock) {
      if (query->wait > 0) {
         cfl_str_appendFormat(str, " for update wait %hu", query->wait);
      } else {
         CFL_STR_APPEND_CONST(str, " for update nowait");
      }
   }
}

static void query_free(CFL_SQLP sql) {
   CFL_SQL_QUERYP query = (CFL_SQL_QUERYP) sql;
   if (query->hintValue != NULL) {
      cfl_str_free(query->hintValue);
   }
   if (query->columns != NULL) {
      list_free(query->columns);
   }
   if (query->datasets != NULL) {
      list_free(query->datasets);
   }
   if (query->condition != NULL) {
      query->condition->free_sql(query->condition);
   }
   if (query->orders != NULL) {
      list_free(query->orders);
   }
   CFL_MEM_FREE(query);
}

static CFL_SQL_QUERYP query_hint(CFL_SQL_QUERYP query, CFL_STRP value) {
   query->hintValue = cfl_str_setStr(query->hintValue, value);
   return query;
}

static CFL_SQL_QUERYP query_select(CFL_SQL_QUERYP query, CFL_SQLP first, ...) {
   va_list args;
   va_start(args, first);
   query->columns = list_add_items(query->columns, first, args);
   va_end(args);
   return query;
}

static CFL_SQL_QUERYP query_from(CFL_SQL_QUERYP query, CFL_SQLP first, ...) {
   va_list args;
   va_start(args, first);
   query->datasets = list_add_items(query->datasets, first, args);
   va_end(args);
   return query;
}

static CFL_SQL_QUERYP query_where(CFL_SQL_QUERYP query, CFL_SQLP condition) {
   if (query->condition != NULL) {
      query->condition->free_sql(query->condition);
   }
   query->condition = condition;
   return query;
}

static CFL_SQL_QUERYP query_order_by(CFL_SQL_QUERYP query, CFL_SQLP first, ...) {
   va_list args;
   va_start(args, first);
   query->orders = list_add_items(query->orders, first, args);
   va_end(args);
   return query;
}

static CFL_SQL_QUERYP query_for_update(CFL_SQL_QUERYP query, CFL_UINT16 wait) {
   query->lock = CFL_TRUE;
   query->wait = wait;
   return query;
}

static CFL_SQL_QUERYP query_new(void) {
   CFL_SQL_QUERYP query = (CFL_SQL_QUERYP) CFL_MEM_ALLOC(sizeof(CFL_SQL_QUERY));
   query->sql.to_string = query_to_string;
   query->sql.free_sql = query_free;
   query->hintValue = NULL;
   query->columns = NULL;
   query->datasets = NULL;
   query->condition = NULL;
   query->orders = NULL;
   query->lock = CFL_FALSE;
   query->wait = 0;
   query->hint = query_hint;
   query->select = query_select;
   query->from = query_from;
   query->where = query_where;
   query->orderBy = query_order_by;
   query->forUpdate = query_for_update;
   return query;
}

/***************************
 * INSERT
 ***************************/
static void insert_to_string(CFL_SQLP sql, CFL_STRP str) {
   CFL_SQL_INSERTP ins = (CFL_SQL_INSERTP) sql;
   CFL_STR_APPEND_CONST(str, "insert into ");
   ins->tableName->to_string(ins->tableName, str);
   if (ins->columnsNames != NULL && cfl_list_length(ins->columnsNames) > 0) {
      cfl_str_appendChar(str, '(');
      list_to_string(ins->columnsNames, str, ", ");
      cfl_str_appendChar(str, ')');
   }
   CFL_STR_APPEND_CONST(str, " values(");
   list_to_string(ins->columnsValues, str, ", ");
   cfl_str_appendChar(str, ')');
   if (ins->returningColumns != NULL && cfl_list_length(ins->returningColumns) > 0) {
      CFL_STR_APPEND_CONST(str, " returning ");
      list_to_string(ins->returningColumns, str, ", ");
      CFL_STR_APPEND_CONST(str, " into ");
      list_to_string(ins->returningParams, str, ", ");
   }
}

static void insert_free(CFL_SQLP sql) {
   CFL_SQL_INSERTP ins = (CFL_SQL_INSERTP) sql;
   if (ins->tableName != NULL) {
      ins->tableName->free_sql(ins->tableName);
   }
   if (ins->columnsNames != NULL) {
      list_free(ins->columnsNames);
   }
   if (ins->columnsValues != NULL) {
      list_free(ins->columnsValues);
   }
   if (ins->returningColumns != NULL) {
      list_free(ins->returningColumns);
   }
   if (ins->returningParams != NULL) {
      list_free(ins->returningParams);
   }
   CFL_MEM_FREE(ins);
}

static CFL_SQL_INSERTP insert_into(CFL_SQL_INSERTP ins, CFL_SQLP table) {
   if (ins->tableName != NULL) {
      ins->tableName->free_sql(ins->tableName);
   }
   ins->tableName = table;
   return ins;
}

static CFL_SQL_INSERTP insert_columns(CFL_SQL_INSERTP ins, CFL_SQLP first, ...) {
   va_list args;
   va_start(args, first);
   ins->columnsNames = list_add_items(ins->columnsNames, first, args);
   va_end(args);
   return ins;
}

static CFL_SQL_INSERTP insert_values(CFL_SQL_INSERTP ins, CFL_SQLP first, ...) {
   va_list args;
   va_start(args, first);
   ins->columnsValues = list_add_items(ins->columnsValues, first, args);
   va_end(args);
   return ins;
}

static CFL_SQL_INSERTP insert_returning(CFL_SQL_INSERTP ins, CFL_SQLP col, CFL_SQLP param) {
   if (ins->returningColumns == NULL) {
      ins->returningColumns = cfl_list_new(2);
   }
   if (ins->returningParams == NULL) {
      ins->returningParams = cfl_list_new(2);
   }
   cfl_list_add(ins->returningColumns, col);
   cfl_list_add(ins->returningParams, param);
   return ins;
}

static CFL_SQL_INSERTP insert_new(void) {
   CFL_SQL_INSERTP ins = (CFL_SQL_INSERTP) CFL_MEM_ALLOC(sizeof(CFL_SQL_INSERT));
   ins->sql.to_string = insert_to_string;
   ins->sql.free_sql = insert_free;
   ins->tableName = NULL;
   ins->columnsNames = NULL;
   ins->columnsValues = NULL;
   ins->returningColumns = NULL;
   ins->returningParams = NULL;
   ins->into = insert_into;
   ins->columns = insert_columns;
   ins->values = insert_values;
   ins->returning = insert_returning;
   return ins;
}

/***************************
 * UPDATE
 ***************************/
static void update_to_string(CFL_SQLP sql, CFL_STRP str) {
   CFL_SQL_UPDATEP upd = (CFL_SQL_UPDATEP) sql;
   CFL_STR_APPEND_CONST(str, "update ");
   upd->tableName->to_string(upd->tableName, str);
   CFL_STR_APPEND_CONST(str, " set ");
   if (upd->columnsNames != NULL && upd->columnsValues != NULL) {
      double_list_to_string(upd->columnsNames, upd->columnsValues, str, "=", ", ");
   }
   if (upd->condition != NULL) {
      CFL_STR_APPEND_CONST(str, " where ");
      upd->condition->to_string(upd->condition, str);
   }
   if (upd->returningColumns != NULL && cfl_list_length(upd->returningColumns) > 0) {
      CFL_STR_APPEND_CONST(str, " returning ");
      list_to_string(upd->returningColumns, str, ", ");
      CFL_STR_APPEND_CONST(str, " into ");
      list_to_string(upd->returningParams, str, ", ");
   }
}

static void update_free(CFL_SQLP sql) {
   CFL_SQL_UPDATEP upd = (CFL_SQL_UPDATEP) sql;
   if (upd->tableName != NULL) {
      upd->tableName->free_sql(upd->tableName);
   }
   if (upd->columnsNames != NULL) {
      list_free(upd->columnsNames);
   }
   if (upd->columnsValues != NULL) {
      list_free(upd->columnsValues);
   }
   if (upd->condition != NULL) {
      upd->condition->free_sql(upd->condition);
   }
   if (upd->returningColumns != NULL) {
      list_free(upd->returningColumns);
   }
   if (upd->returningParams != NULL) {
      list_free(upd->returningParams);
   }
   CFL_MEM_FREE(upd);
}

static CFL_SQL_UPDATEP update_table(CFL_SQL_UPDATEP upd, CFL_SQLP table) {
   if (upd->tableName != NULL) {
      upd->tableName->free_sql(upd->tableName);
   }
   upd->tableName = table;
   return upd;
}

static CFL_SQL_UPDATEP update_set(CFL_SQL_UPDATEP upd, CFL_SQLP col, CFL_SQLP value) {
   if (upd->columnsNames == NULL) {
      upd->columnsNames = cfl_list_new(2);
   }
   if (upd->columnsValues == NULL) {
      upd->columnsValues = cfl_list_new(2);
   }
   cfl_list_add(upd->columnsNames, col);
   cfl_list_add(upd->columnsValues, value);
   return upd;
}

static CFL_SQL_UPDATEP update_where(CFL_SQL_UPDATEP upd, CFL_SQLP condition) {
   if (upd->condition != NULL) {
      upd->condition->free_sql(upd->condition);
   }
   upd->condition = condition;
   return upd;
}

static CFL_SQL_UPDATEP update_returning(CFL_SQL_UPDATEP upd, CFL_SQLP col, CFL_SQLP param) {
   if (upd->returningColumns == NULL) {
      upd->returningColumns = cfl_list_new(2);
   }
   if (upd->returningParams == NULL) {
      upd->returningParams = cfl_list_new(2);
   }
   cfl_list_add(upd->returningColumns, col);
   cfl_list_add(upd->returningParams, param);
   return upd;
}

static CFL_SQL_UPDATEP update_new(void) {
   CFL_SQL_UPDATEP upd = (CFL_SQL_UPDATEP) CFL_MEM_ALLOC(sizeof(CFL_SQL_UPDATE));
   upd->sql.to_string = update_to_string;
   upd->sql.free_sql = update_free;
   upd->tableName = NULL;
   upd->columnsNames = NULL;
   upd->columnsValues = NULL;
   upd->condition = NULL;
   upd->returningColumns = NULL;
   upd->returningParams = NULL;
   upd->table = update_table;
   upd->set = update_set;
   upd->where = update_where;
   upd->returning = update_returning;
   return upd;
}

/***************************
 * DELETE
 ***************************/
static void delete_to_string(CFL_SQLP sql, CFL_STRP str) {
   CFL_SQL_DELETEP del = (CFL_SQL_DELETEP) sql;
   CFL_STR_APPEND_CONST(str, "delete from ");
   del->tableName->to_string(del->tableName, str);
   if (del->condition != NULL) {
      CFL_STR_APPEND_CONST(str, " where ");
      del->condition->to_string(del->condition, str);
   }
   if (del->returningColumns != NULL && cfl_list_length(del->returningColumns) > 0) {
      CFL_STR_APPEND_CONST(str, " returning ");
      list_to_string(del->returningColumns, str, ", ");
      CFL_STR_APPEND_CONST(str, " into ");
      list_to_string(del->returningParams, str, ", ");
   }
}

static void delete_free(CFL_SQLP sql) {
   CFL_SQL_DELETEP del = (CFL_SQL_DELETEP) sql;
   if (del->tableName != NULL) {
      del->tableName->free_sql(del->tableName);
   }
   if (del->condition != NULL) {
      del->condition->free_sql(del->condition);
   }
   if (del->returningColumns != NULL) {
      list_free(del->returningColumns);
   }
   if (del->returningParams != NULL) {
      list_free(del->returningParams);
   }
   CFL_MEM_FREE(del);
}

static CFL_SQL_DELETEP delete_from(CFL_SQL_DELETEP del, CFL_SQLP table) {
   if (del->tableName != NULL) {
      del->tableName->free_sql(del->tableName);
   }
   del->tableName = table;
   return del;
}

static CFL_SQL_DELETEP delete_where(CFL_SQL_DELETEP del, CFL_SQLP condition) {
   if (del->condition != NULL) {
      del->condition->free_sql(del->condition);
   }
   del->condition = condition;
   return del;
}

static CFL_SQL_DELETEP delete_returning(CFL_SQL_DELETEP del, CFL_SQLP col, CFL_SQLP param) {
   if (del->returningColumns == NULL) {
      del->returningColumns = cfl_list_new(2);
   }
   if (del->returningParams == NULL) {
      del->returningParams = cfl_list_new(2);
   }
   cfl_list_add(del->returningColumns, col);
   cfl_list_add(del->returningParams, param);
   return del;
}

static CFL_SQL_DELETEP delete_new(void) {
   CFL_SQL_DELETEP del = (CFL_SQL_DELETEP) CFL_MEM_ALLOC(sizeof(CFL_SQL_DELETE));
   del->sql.to_string = delete_to_string;
   del->sql.free_sql = delete_free;
   del->tableName = NULL;
   del->condition = NULL;
   del->returningColumns = NULL;
   del->returningParams = NULL;
   del->from = delete_from;
   del->where = delete_where;
   del->returning = delete_returning;
   return del;
}

/***************************
 * EXPRESSION
 ***************************/
static void custom_to_string(CFL_SQLP sql, CFL_STRP str) {
   cfl_str_appendStr(str, ((CFL_SQL_CUSTOMP) sql)->value);
}

static void param_to_string(CFL_SQLP sql, CFL_STRP str) {
   cfl_str_appendChar(str, ':');
   cfl_str_appendStr(str, ((CFL_SQL_CUSTOMP) sql)->value);
}

static void custom_free(CFL_SQLP sql) {
   if (((CFL_SQL_CUSTOMP) sql)->value != NULL) {
      cfl_str_free(((CFL_SQL_CUSTOMP) sql)->value);
   }   
   CFL_MEM_FREE(sql);
}

static CFL_SQLP quali_id_new(CFL_STRP first, ...) {
   CFL_SQL_CUSTOMP id = (CFL_SQL_CUSTOMP) CFL_MEM_ALLOC(sizeof(CFL_SQL_CUSTOM));
   CFL_STRP item;
   va_list args;
   
   id->sql.to_string = custom_to_string;
   id->sql.free_sql = custom_free;
   id->value = cfl_str_newStr(first);
   va_start(args, first);
   item = va_arg(args, CFL_STRP);
   while ( item != NULL ) {
      cfl_str_appendChar(id->value, '.');
      cfl_str_appendStr(id->value, item);
      item = va_arg(args, CFL_STRP);
   }
   va_end(args);
   return (CFL_SQLP) id;
}

static CFL_SQLP c_quali_id_new(char *first, ...) {
   CFL_SQL_CUSTOMP id = (CFL_SQL_CUSTOMP) CFL_MEM_ALLOC(sizeof(CFL_SQL_CUSTOM));
   char *item;
   va_list args;
   
   id->sql.to_string = custom_to_string;
   id->sql.free_sql = custom_free;
   id->value = cfl_str_newBuffer(first);
   va_start(args, first);
   item = va_arg(args, char *);
   while ( item != NULL ) {
      cfl_str_appendChar(id->value, '.');
      cfl_str_append(id->value, item, NULL);
      item = va_arg(args, char *);
   }
   va_end(args);
   return (CFL_SQLP) id;
}

/***************************
 * CUSTOM
 ***************************/
static void pragma_to_string(CFL_SQLP sql, CFL_STRP str) {
   CFL_STR_APPEND_CONST(str, "pragma ");
   cfl_str_appendStr(str, ((CFL_SQL_CUSTOMP) sql)->value);
}

CONST_CUSTOM_SQL(commit_new, "commit")
CONST_CUSTOM_SQL(rollback_new, "rollback")
CONST_CUSTOM_SQL(null_new, "null")
CUSTOM_SQL2(param_new, param_to_string)
C_CUSTOM_SQL2(c_param_new, param_to_string)
CUSTOM_SQL(custom_new)
C_CUSTOM_SQL(c_custom_new)
CUSTOM_SQL2(pragma_new, pragma_to_string)


/***************************
 * DOUBLE ARG OPERATOR 
 ***************************/
static void double_op_to_string(CFL_SQLP sql, CFL_STRP str) {
   ((CFL_SQL_DOUBLE_OPP) sql)->left->to_string(((CFL_SQL_DOUBLE_OPP) sql)->left, str);
   cfl_str_appendStr(str, ((CFL_SQL_DOUBLE_OPP) sql)->op);
   ((CFL_SQL_DOUBLE_OPP) sql)->right->to_string(((CFL_SQL_DOUBLE_OPP) sql)->right, str);
}

static void double_op_free(CFL_SQLP sql) {
   if (((CFL_SQL_DOUBLE_OPP) sql)->op != NULL) {
      cfl_str_free(((CFL_SQL_DOUBLE_OPP) sql)->op);
   }   
   if (((CFL_SQL_DOUBLE_OPP) sql)->left != NULL) {
      ((CFL_SQL_DOUBLE_OPP) sql)->left->free_sql(((CFL_SQL_DOUBLE_OPP) sql)->left);
   }   
   if (((CFL_SQL_DOUBLE_OPP) sql)->right != NULL) {
      ((CFL_SQL_DOUBLE_OPP) sql)->right->free_sql(((CFL_SQL_DOUBLE_OPP) sql)->right);
   }   
   CFL_MEM_FREE(sql);
}

DOUBLE_OP(equal_new, "=")
DOUBLE_OP(diff_new, "<>")
DOUBLE_OP(in_new, " in ")
DOUBLE_OP(not_in_new, " not in ")
DOUBLE_OP(exists_new, " exists ")
DOUBLE_OP(not_exists_new, " not exists ")
DOUBLE_OP(like_new, " like ")
DOUBLE_OP(greater_new, ">")
DOUBLE_OP(less_new, "<")
DOUBLE_OP(greater_equal_new, ">=")
DOUBLE_OP(less_equal_new, "<=")
DOUBLE_OP(plus_new, "+")
DOUBLE_OP(minus_new, "-")
DOUBLE_OP(div_new, "/")
DOUBLE_OP(mult_new, "*")

static CFL_SQLP and_new(CFL_SQLP left, CFL_SQLP right) {
   if (left != NULL) {
      if (right != NULL) {
         CFL_SQL_DOUBLE_OPP newOp = (CFL_SQL_DOUBLE_OPP) CFL_MEM_ALLOC(sizeof(CFL_SQL_DOUBLE_OP));
         newOp->sql.to_string = double_op_to_string;
         newOp->sql.free_sql = double_op_free;
         newOp->left = left;
         newOp->op = cfl_str_newConst(" and ");
         newOp->right = right;
         return (CFL_SQLP) newOp;
      } else {
         return left;
      }
   } else {
      return right;
   } 
}

static CFL_SQLP or_new(CFL_SQLP left, CFL_SQLP right) {
   if (left != NULL) {
      if (right != NULL) {
         CFL_SQL_DOUBLE_OPP newOp = (CFL_SQL_DOUBLE_OPP) CFL_MEM_ALLOC(sizeof(CFL_SQL_DOUBLE_OP));
         newOp->sql.to_string = double_op_to_string;
         newOp->sql.free_sql = double_op_free;
         newOp->left = left;
         newOp->op = cfl_str_newConst(" or ");
         newOp->right = right;
         return (CFL_SQLP) newOp;
      } else {
         return left;
      }
   } else {
      return right;
   } 
}


/***************************
 * SINGLE ARG OPERATOR 
 ***************************/
static void pos_op_to_string(CFL_SQLP sql, CFL_STRP str) {
   ((CFL_SQL_SINGLE_OPP) sql)->expr->to_string(((CFL_SQL_SINGLE_OPP) sql)->expr, str);
   cfl_str_appendChar(str, ' ');
   cfl_str_appendStr(str, ((CFL_SQL_SINGLE_OPP) sql)->op);
}

static void single_op_free(CFL_SQLP sql) {
   if (((CFL_SQL_SINGLE_OPP) sql)->op != NULL) {
      cfl_str_free(((CFL_SQL_SINGLE_OPP) sql)->op);
   }   
   if (((CFL_SQL_SINGLE_OPP) sql)->expr != NULL) {
      ((CFL_SQL_SINGLE_OPP) sql)->expr->free_sql(((CFL_SQL_SINGLE_OPP) sql)->expr);
   }   
   CFL_MEM_FREE(sql);
}

POS_OP(is_null_new, "is null")
POS_OP(is_not_null_new, "is not null")

static CFL_SQLP expr_alias_new(CFL_SQLP expr, CFL_STRP alias) {
   CFL_SQL_SINGLE_OPP newOp = (CFL_SQL_SINGLE_OPP) CFL_MEM_ALLOC(sizeof(CFL_SQL_SINGLE_OP));
   newOp->sql.to_string = pos_op_to_string;
   newOp->sql.free_sql = single_op_free;
   newOp->expr = expr;
   newOp->op = cfl_str_newStr(alias);
   return (CFL_SQLP) newOp;
}

/***************************
 * WRAPPER 
 ***************************/
static void wrap_free(CFL_SQLP sql) {
   if (((CFL_SQL_WRAPP) sql)->expr != NULL) {
      ((CFL_SQL_WRAPP) sql)->expr->free_sql(((CFL_SQL_WRAPP) sql)->expr);
   }   
   CFL_MEM_FREE(sql);
}

static void par_to_string(CFL_SQLP sql, CFL_STRP str) {
   cfl_str_appendChar(str, '(');
   ((CFL_SQL_WRAPP) sql)->expr->to_string(((CFL_SQL_WRAPP) sql)->expr, str);
   cfl_str_appendChar(str, ')');
}

static void desc_to_string(CFL_SQLP sql, CFL_STRP str) {
   ((CFL_SQL_WRAPP) sql)->expr->to_string(((CFL_SQL_WRAPP) sql)->expr, str);
   CFL_STR_APPEND_CONST(str, " desc");
}

static void asc_to_string(CFL_SQLP sql, CFL_STRP str) {
   ((CFL_SQL_WRAPP) sql)->expr->to_string(((CFL_SQL_WRAPP) sql)->expr, str);
   CFL_STR_APPEND_CONST(str, " asc");
}

static CFL_SQLP parentheses_new(CFL_SQLP expr) {
   CFL_SQL_WRAPP newOp = (CFL_SQL_WRAPP) CFL_MEM_ALLOC(sizeof(CFL_SQL_WRAP));
   newOp->sql.to_string = par_to_string;
   newOp->sql.free_sql = wrap_free;
   newOp->expr = expr;
   return (CFL_SQLP) newOp;
}

static CFL_SQLP desc_new(CFL_SQLP expr) {
   CFL_SQL_WRAPP newOp = (CFL_SQL_WRAPP) CFL_MEM_ALLOC(sizeof(CFL_SQL_WRAP));
   newOp->sql.to_string = desc_to_string;
   newOp->sql.free_sql = wrap_free;
   newOp->expr = expr;
   return (CFL_SQLP) newOp;
}

static CFL_SQLP asc_new(CFL_SQLP expr) {
   CFL_SQL_WRAPP newOp = (CFL_SQL_WRAPP) CFL_MEM_ALLOC(sizeof(CFL_SQL_WRAP));
   newOp->sql.to_string = asc_to_string;
   newOp->sql.free_sql = wrap_free;
   newOp->expr = expr;
   return (CFL_SQLP) newOp;
}

/***************************
 * FUNCTION CALL
 ***************************/
static void fun_to_string(CFL_SQLP sql, CFL_STRP str) {
   cfl_str_appendStr(str, ((CFL_SQL_FUNP) sql)->name);
   if (((CFL_SQL_FUNP) sql)->args != NULL && cfl_list_length(((CFL_SQL_FUNP) sql)->args) > 0) {
      cfl_str_appendChar(str, '(');
      list_to_string(((CFL_SQL_FUNP) sql)->args, str, ", ");
      cfl_str_appendChar(str, ')');
   }
}

static void fun_free(CFL_SQLP sql) {
   if (((CFL_SQL_FUNP) sql)->name != NULL) {
      cfl_str_free(((CFL_SQL_FUNP) sql)->name);
   }
   if (((CFL_SQL_FUNP) sql)->args != NULL) {
      list_free(((CFL_SQL_FUNP) sql)->args);
   }
   CFL_MEM_FREE(sql);
}

static CFL_SQLP fun_new(CFL_STRP funName, ...) {
   CFL_SQL_FUNP fun = (CFL_SQL_FUNP) CFL_MEM_ALLOC(sizeof(CFL_SQL_FUN));
   CFL_SQLP item;
   va_list args;
   
   fun->sql.to_string = fun_to_string;
   fun->sql.free_sql = fun_free;
   fun->name = cfl_str_newStr(funName);
   va_start(args, funName);
   item = va_arg(args, CFL_SQLP);
   if (item != NULL) {
      ((CFL_SQL_FUNP) fun)->args = cfl_list_new(3);
      do {
         cfl_list_add(fun->args, item);
         item = va_arg(args, CFL_SQLP);
      } while ( item != NULL );
   } else {
      ((CFL_SQL_FUNP) fun)->args = NULL;
   }
   va_end(args);
   return (CFL_SQLP) fun;
}

static CFL_SQLP c_fun_new(char *funName, ...) {
   CFL_SQL_FUNP fun = (CFL_SQL_FUNP) CFL_MEM_ALLOC(sizeof(CFL_SQL_FUN));
   CFL_SQLP item;
   va_list args;
   
   fun->sql.to_string = fun_to_string;
   fun->sql.free_sql = fun_free;
   fun->name = cfl_str_newBuffer(funName);
   va_start(args, funName);
   item = va_arg(args, CFL_SQLP);
   if (item != NULL) {
      ((CFL_SQL_FUNP) fun)->args = cfl_list_new(3);
      do {
         cfl_list_add(fun->args, item);
         item = va_arg(args, CFL_SQLP);
      } while ( item != NULL );
   } else {
      ((CFL_SQL_FUNP) fun)->args = NULL;
   }
   va_end(args);
   return (CFL_SQLP) fun;
}

static CFL_SQLP format_new(const char *format, ...) {
   CFL_SQL_CUSTOMP custom = (CFL_SQL_CUSTOMP) CFL_MEM_ALLOC(sizeof(CFL_SQL_CUSTOM));
   va_list args;
   custom->sql.to_string = custom_to_string;
   custom->sql.free_sql = custom_free;
   custom->value = cfl_str_new(16);
   va_start(args, format);
   cfl_str_appendFormatArgs(custom->value, format, args);
   va_end(args);
   return (CFL_SQLP) custom;
}


/***************************
 * BLOCK
 ***************************/
static void block_to_string(CFL_SQLP sql, CFL_STRP str) {
   CFL_SQL_BLOCKP block = (CFL_SQL_BLOCKP) sql;
   CFL_STR_APPEND_CONST(str, "declare\r");
   if (block->pragmas != NULL) {
      list_to_string(block->pragmas, str, ";\r");
   }
   if (block->vars != NULL) {
      list_to_string(block->vars, str, ";\r");
   }
   CFL_STR_APPEND_CONST(str, "begin\r");
   if (block->statements != NULL) {
      list_to_string(block->statements, str, ";\r");
   }
   CFL_STR_APPEND_CONST(str, "end;");
}

static void block_free(CFL_SQLP sql) {
   CFL_SQL_BLOCKP block = (CFL_SQL_BLOCKP) sql;
   if (block->pragmas != NULL) {
      list_free(block->pragmas);
   }
   if (block->vars != NULL) {
      list_free(block->vars);
   }
   if (block->statements != NULL) {
      list_free(block->statements);
   }
   CFL_MEM_FREE(block);
}

static void var_to_string(CFL_SQLP sql, CFL_STRP str) {
   cfl_str_appendStr(str, ((CFL_SQL_DOUBLE_OPP) sql)->op);
   if (((CFL_SQL_DOUBLE_OPP) sql)->left != NULL) {
      cfl_str_appendChar(str, ' ');
      ((CFL_SQL_DOUBLE_OPP) sql)->left->to_string(((CFL_SQL_DOUBLE_OPP) sql)->left, str);
   }
   if (((CFL_SQL_DOUBLE_OPP) sql)->right != NULL) {
      CFL_STR_APPEND_CONST(str, ":=");
      ((CFL_SQL_DOUBLE_OPP) sql)->right->to_string(((CFL_SQL_DOUBLE_OPP) sql)->right, str);
   }
}

static CFL_SQLP var_new(CFL_STRP varName, CFL_SQLP varType, CFL_SQLP varValue) {
   CFL_SQL_DOUBLE_OPP varSql = (CFL_SQL_DOUBLE_OPP) CFL_MEM_ALLOC(sizeof(CFL_SQL_DOUBLE_OP));
   varSql->sql.to_string = var_to_string;
   varSql->sql.free_sql = double_op_free;
   varSql->op = cfl_str_newStr(varName);
   varSql->left = varType;
   varSql->right = varValue;
   return (CFL_SQLP) varSql;
   
}

static CFL_SQL_BLOCKP block_var(CFL_SQL_BLOCKP block, CFL_STRP varName, CFL_SQLP varType, CFL_SQLP varValue) {
   if (block->vars == NULL) {
      block->vars = cfl_list_new(2);
   }
   cfl_list_add(block->vars, var_new(varName, varType, varValue));
   return block;
}

static CFL_SQL_BLOCKP block_pragma(CFL_SQL_BLOCKP block, CFL_STRP pragma) {
   if (block->pragmas == NULL) {
      block->pragmas = cfl_list_new(2);
   }
   cfl_list_add(block->pragmas, pragma_new(pragma));
   return block;
}

static CFL_SQL_BLOCKP block_statement(CFL_SQL_BLOCKP block, CFL_SQLP stmt) {
   if (block->statements == NULL) {
      block->statements = cfl_list_new(2);
   }
   cfl_list_add(block->statements, stmt);
   return block;
}

static CFL_SQL_BLOCKP block_new(void) {
   CFL_SQL_BLOCKP block = (CFL_SQL_BLOCKP) CFL_MEM_ALLOC(sizeof(CFL_SQL_BLOCK));
   block->sql.to_string = block_to_string;
   block->sql.free_sql = block_free;
   block->vars = NULL;
   block->pragmas = NULL;
   block->statements = NULL;
   block->var = block_var;
   block->pragma = block_pragma;
   block->statement = block_statement;
   return block;
}

void cfl_sql_initBuilder(CFL_SQL_BUILDERP builder) {
   builder->query = query_new;
   builder->insert = insert_new;
   builder->update = update_new;
   builder->delete = delete_new;
   builder->id = custom_new;
   builder->quali_id = quali_id_new;
   builder->param = param_new;
   builder->c_id = c_custom_new;
   builder->c_quali_id = c_quali_id_new;
   builder->c_param = c_param_new;
   builder->as = expr_alias_new;
   builder->equal = equal_new;
   builder->diff = diff_new;
   builder->in = in_new;
   builder->notIn = not_in_new;
   builder->exists = exists_new;
   builder->notExists = not_exists_new;
   builder->like = like_new;
   builder->greater= greater_new;
   builder->less = less_new;
   builder->greaterEqual = greater_equal_new;
   builder->lessEqual = less_equal_new;
   builder->isNull = is_null_new;
   builder->isNotNull = is_not_null_new;
   builder->and = and_new;
   builder->or = or_new;
   builder->parentheses = parentheses_new;
   builder->desc = desc_new;
   builder->asc = asc_new;
   builder->fun = fun_new;
   builder->c_fun = c_fun_new;
   builder->custom = custom_new;
   builder->c_custom = c_custom_new;
   builder->format = format_new;
   builder->plus = plus_new;
   builder->minus = minus_new;
   builder->div = div_new;
   builder->mult = mult_new;
   builder->commit = commit_new;
   builder->rollback = rollback_new;
   builder->null = null_new;
   builder->block = block_new;
}
