/*
 *  Copyright (c) 2016, https://github.com/zhatalk
 *  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "nebula/storage/mysql/mysql_database.h"

#include <iostream>
#include <mysqld_error.h>

#include <folly/Format.h>
#include <folly/Conv.h>

#include "nebula/base/logger/glog_util.h"

// #include "base/string_util.h"
// #include "base/strings/stringprintf.h"
// #include "base/strings/string_number_conversions.h"
// #include "base/memory/scoped_ptr.h"

#define CHECK_IS_FETCH() if (!is_fetched_) { \
  LOG(ERROR) << "must first call FetchRow()!!!!!"; \
  return false; \
}

namespace {
  
const char kBeginTransaction[] = "START TRANSACTION";
const char kCommitTransaction[] = "COMMIT";
const char kRollbackTransaction[] = "ROLLBACK";

const char kSetNamesUTF8[] = "SET NAMES `utf8`";
const char kSetCharactesUTF8[] = "SET CHARACTER SET `utf8`";
  
}

namespace db {
  
//------------------------------------------------------------------------
class MySQLAnswer
  : public QueryAnswer {
public:
  MySQLAnswer(MYSQL_RES *res, uint32_t columns, uint64_t rows)
    : QueryAnswer() {
    is_fetched_ = false;
    lengths_ = NULL;
    row_ = NULL;
    res_ = res;
    column_count_ = columns;
    row_count_ = rows;
    fields_ = res->fields;
  }
    
  virtual ~MySQLAnswer() {
    if (res_) {
      mysql_free_result(res_);
    }
    res_ = 0;
    lengths_ = NULL;
  }
  
  virtual bool FetchRow() {
    if (!is_fetched_) is_fetched_ = true;
    if (res_ && (row_ = mysql_fetch_row(res_))) {
      lengths_ = mysql_fetch_lengths(res_);
      return true;
    } else {
      return false;
    }
  }
  
  virtual void PrintDebugFieldNames() const {
    for (uint32_t i=0; i<column_count_; ++i) {
      std::cout << fields_[i].name << std::endl;
    }
  }
  
  const char* GetColumnName(uint32_t clm) const {
    CHECK(clm < column_count_);
    return fields_[clm].name;
  }
    
  
  virtual bool GetColumn(uint32_t clm, std::string* val) const {
    CHECK_IS_FETCH();
    
    CHECK(clm < column_count_);
    if(row_[clm]) {
      val->clear();
      val->assign(row_[clm], lengths_[clm]);
    } else {
      
    }
    return true;
  }
  
  virtual uint32_t GetColumnLength(uint32_t clm) const {
    CHECK_IS_FETCH();

    CHECK(clm < column_count_);
    return static_cast<uint32_t>(lengths_[clm]);
  }
  
  virtual bool ColumnIsNull(uint32_t clm) const {
    CHECK_IS_FETCH();

    return row_[clm]==NULL;
  }
  
  
  virtual bool GetColumn(uint32_t clm, uint64_t* val) const {
    CHECK_IS_FETCH();

    CHECK(clm < column_count_);
    if (row_[clm]) {
      // int64 val2;
      // bool ret = base::StringToInt64(row_[clm], &val2);
      // if (ret) *val = (uint64)(val2);
      // return ret;
 
      *val = folly::to<uint64_t>(row_[clm]);
      return true;
    } else {
      *val = 0;
      return false;
    }
  }
  
  virtual bool GetColumn(uint32_t clm, int64_t* val) const {
    CHECK_IS_FETCH();

    CHECK(clm < column_count_);
    if (row_[clm]) {
      // return base::StringToInt64(row_[clm], val);
      
      *val = folly::to<int64_t>(row_[clm]);
      return true;
    } else {
      *val = 0;
      return false;
    }
  }
  
  virtual bool GetColumn(uint32_t clm, uint32_t *val) const {
    CHECK_IS_FETCH();

    CHECK(clm < column_count_);
    if (row_[clm]) {
      // return base::StringToUint(row_[clm], val);
      
      *val = folly::to<uint32_t>(row_[clm]);
      return true;
    } else {
      *val = 0;
      return false;
    }
  }
  
  virtual bool GetColumn(uint32_t clm, int *val) const {
    CHECK_IS_FETCH();

    CHECK(clm < column_count_);
    if (row_[clm]) {
      // return base::StringToInt(row_[clm], val);
      // *val = atoi(row_[clm]);
      // return true;
      
      *val = folly::to<int>(row_[clm]);
      return true;

    } else {
      *val = 0;
      return false;
    }
  }
  
  virtual bool GetColumn(uint32_t clm, float* val) const {
    CHECK_IS_FETCH();

    CHECK(clm < column_count_);
    if (row_[clm]) {
      // double val2;
      // bool ret = base::StringToDouble(row_[clm], &val2);
      // if (ret) *val = (float)(val2);
      // return ret;
      
      *val = folly::to<float>(row_[clm]);
      return true;

    } else {
      *val = 0.0;
      return false;
    }
  }
  
  virtual bool GetColumn(uint32_t clm, bool* val) const {
    CHECK_IS_FETCH();

    CHECK(clm < column_count_);
    if (row_[clm]) {
      // int val2;
      // bool ret = base::StringToInt(row_[clm], &val2);
      // if (ret) *val = (val2==0 ? false:true);
      // return ret;
      
      int val2 = folly::to<int>(row_[clm]);
      *val = (val2==0 ? false:true);
      return true;

    } else {
      *val = false;
      return false;
    }
  }
  
protected:
  virtual uint32_t GetIndexByFieldName(const folly::StringPiece& name) const {
    uint32_t index = column_count_;
    for (uint32_t i=0; i<column_count_; ++i) {
      if (name.compare(fields_[i].name) == 0) {
        index = i;
        break;
      }
    }
    return index;
  }
    
  MYSQL_RES *res_;
  MYSQL_ROW row_;
  MYSQL_FIELD* fields_;
  unsigned long *lengths_;
  bool is_fetched_;
};

//------------------------------------------------------------------------
MySQLDatabase::MySQLDatabase()
  : my_(NULL) {
}

MySQLDatabase::~MySQLDatabase() {
  CloseDb();
}

QueryAnswer *MySQLDatabase::Query(const folly::StringPiece& q_str) {
  DCHECK(!q_str.empty()) << "Querystring is not empty";
  if (!CheckConnection()) return NULL;
  
  MySQLAnswer *answ = NULL;
  
  int er=0;
  if (SafeQuery(q_str, &er)) {
    LOG(ERROR) << mysql_error(my_) << ". In sql: " << q_str;
  } else {
    MYSQL_RES *res = mysql_store_result(my_);
    uint64_t rows = mysql_affected_rows(my_);
    uint32_t fields = mysql_field_count(my_);
    
    if (res) {
      answ = new MySQLAnswer(res, fields, rows);
      //answ->FetchRow();
    }
    
//    if (res==NULL || rows==0 || fields==0) {
//      if (res != NULL)
//        mysql_free_result(res);
//    } else  {
//      answ = new MySQLAnswer(res, fields, rows);
//      answ->FetchRow();
//    }
  }
  return answ;
}

uint64_t MySQLDatabase::ExecuteInsertID(const folly::StringPiece& q_str) {
  DCHECK(!q_str.empty()) << "Querystring is not empty";
  if (!CheckConnection()) return 0;
  
  uint64_t ret = 0;
  int er=0;
  if (SafeQuery(q_str, &er)) {
    LOG(ERROR) << mysql_error(my_) << ". In sql: " << q_str;
  } else {
    ret = mysql_insert_id(my_);
  }
  
  return ret;
}

int MySQLDatabase::Execute(const folly::StringPiece& q_str) {
  DCHECK(!q_str.empty()) << "Querystring is not empty";
  if (!CheckConnection()) return -2000;
  
  int er=0;
  int res = SafeQuery(q_str, &er);
  if (res) {
    LOG(ERROR) << mysql_error(my_) << ". In sql: " << q_str;
    return -res;
  } else {
    return static_cast<int>(mysql_affected_rows(my_));
  }
}

uint64_t MySQLDatabase::GetNextID(const char* table_name, const char* field_name) {
  DCHECK(table_name);
  DCHECK(field_name);
  
  int64_t result = 0;
  std::string sql_str = folly::sformat("SELECT MAX({}) AS val FROM {}", field_name, table_name);
  std::unique_ptr<QueryAnswer> answ(Query(sql_str));
  if (answ.get()) {
    if (answ->FetchRow()) {
      answ->GetColumn(0, &result);
    }
  }
  return result;

}

bool MySQLDatabase::BeginTransaction() {
  return 0==Execute(kBeginTransaction);
}

bool MySQLDatabase::CommitTransaction() {
  return 0==Execute(kCommitTransaction);
}

bool MySQLDatabase::RollbackTransaction() {
  return 0==Execute(kRollbackTransaction);
}

//bool MySQLDatabase::Open(const base::StringPiece& conn_string) {
//  return BaseDatabase::Open(conn_string);
//}

void MySQLDatabase::CloseDb() {
  if (my_) {
    mysql_close(my_);
    my_ = NULL;
  }
}

int MySQLDatabase::SafeQuery(const folly::StringPiece& q_str, int* err) {
  int res = mysql_real_query(my_, q_str.data(), q_str.size());
  if (res) {
    *err = mysql_errno(my_);
    switch (*err) {
      case 2006:  // Mysql server has gone away
      case 2008:  // Client ran out of memory
      case 2013:  // Lost connection to sql server during query
      case 2055:  // Lost connection to sql server - system error
      case 1053:  // Lost connection to sql server - system error
        // usleep(5000000);
#ifdef OS_WIN
        Sleep(1000);
#else
        usleep(1000 * 1000);
#endif
        if (BuildConnection()) {
          res = mysql_real_query(my_, q_str.data(), q_str.size());
          if (res) *err=mysql_errno(my_);
        }
        break;
    }
  }
  return res;
}

bool MySQLDatabase::CheckConnection() {
  if (my_) return true;
  
  LOG(ERROR) << "Mysql connection not initialized, ready BuildConnection...";
  return BuildConnection();
}

bool MySQLDatabase::BuildConnection() {
  CloseDb();
  
  MYSQL *my_temp;
  my_temp = mysql_init(&mysql_);
  if (!my_temp) {
    LOG(ERROR) << "Could not initialize Mysql connection";
    return false;
  }
  
  // db_addr_.ParseFromConnString(conn_string);
  mysql_options(my_temp, MYSQL_SET_CHARSET_NAME, "utf8");

  my_ = mysql_real_connect(my_temp, db_addr_.host.c_str(), db_addr_.user.c_str(), db_addr_.passwd.c_str(), db_addr_.db_name.c_str(), db_addr_.port, NULL, 0);
  
  if (my_) {
    // LOG(INFO) << "Connected to MySQL database at " << db_addr_.host;
    // LOG(INFO) << "MySQL client library: " <<  mysql_get_client_info();
    // LOG(INFO) << "MySQL server ver: %s " << mysql_get_server_info(my_);
    
    if (!mysql_autocommit(my_, 1)) {
      // LOG(INFO) << "AUTOCOMMIT SUCCESSFULLY SET TO 1";
    } else {
      LOG(ERROR) << "AUTOCOMMIT NOT SET TO 1";
    }
    // set connection properties to UTF8 to properly handle locales for different
    // server configs - core sends data in UTF8, so MySQL must expect UTF8 too
    Execute(kSetNamesUTF8);
    Execute(kSetCharactesUTF8);
    
#if MYSQL_VERSION_ID >= 50003
    my_bool my_true = (my_bool)1;
    if (mysql_options(my_, MYSQL_OPT_RECONNECT, &my_true)) {
      LOG(ERROR) << "Failed to turn on MYSQL_OPT_RECONNECT.";
    } else {
      // LOG(INFO) << "Successfully turned on MYSQL_OPT_RECONNECT.";
    }
#else
#warning "Your mySQL client lib version does not support reconnecting after a timeout.\nIf this causes you any trouble we advice you to upgrade your mySQL client libs to at least mySQL 5.0.13 to resolve this problem."
#endif
    return true;
    
  } else {
    LOG(ERROR) << "Could not connect to MySQL database at "
    << db_addr_.host
    << ": "
    << mysql_error(my_temp);
    //mysql_close(my_temp);
    return false;
  }
}

bool MySQLDatabase::Ping() {
  int er = mysql_ping(my_);
  if (er != 0) {
    
    // rv = _Reconnect(static_cast<MySQLDatabaseConnection*>(con_ptr));
  }
  return er==0;
}

int MySQLDatabase::GetLastError() {
  if (my_) {
    return mysql_errno(my_);
  } else {
    return mysql_errno(&mysql_);
  }
}

}
