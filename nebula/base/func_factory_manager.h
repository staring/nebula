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

#ifndef NEBULA_BASE_FUNC_FACTORY_MANAGER_H_
#define NEBULA_BASE_FUNC_FACTORY_MANAGER_H_

#include <string>
#include <unordered_map>

#include "nebula/base/logger/glog_util.h"

namespace nebula {
  
//////////////////////////////////////////////////////////////////////////
// TODO(@wubenqi):
//   1. SelfFactoryFactoryManager集成
//   2. Execute返回时使用func返回值
//
// C++11实现的
// 遵循了开闭原则，简洁而优雅的自动注册的对象工厂
// C++11确实能节省很多代码量
template<typename F, typename K=std::string>
class FuncFactoryManager {
public:
  // typedef typename std::result_of<FUNC()>::type::value_type T;
  // typedef typename std::result_of<F()>::type Result;
  
  ~FuncFactoryManager() {}
  
  // 辅助自注册帮助类
  // 只有一个构造函数
  // 通过REGISTER_OBJ宏使用
  struct RegisterTemplate {
    RegisterTemplate(const K& k, F f) {
      // C++11的一个新特性：内部类可以通过外部类的实例访问外部类的私有成员
      // 所以RegisterTemplate可以直接访问SelfRegisterFactoryManager的私有变量factories_
      auto& factories = FuncFactoryManager<F, K>::GetInstance().factories_;
      auto it = factories.find(k);
      if (it != factories.end()) {
        // TODO(@benqi): 是否需要抛出异常？
        LOG(ERROR) << "RegisterTemplate - duplicate entry for key: " << k;
      } else {
        // lambda表达式
        factories.emplace(k, f);
      }
    }
  };
  
  // 执行func
  // TODO(@wubenqi): return值使用func返回值，如果未找到，则需要抛出异常，由上层处理
  template<typename... Args>
  static bool Execute(const K& k, Args... args) {
    auto& factories = GetInstance().factories_;
    auto it = factories.find(k);
    if (it == factories.end()) {
      // TODO(@benqi): 是否需要抛出异常？
      LOG(ERROR) << "CreateInstance - not exist func key: " << k;
      return false;
    } else {
      it->second(args...);
      return true;
    }
  }

  // 执行类成员函数
  template<typename C, typename... Args>
  static bool Execute2(C* c, const K& k, Args... args) {
    auto& factories = GetInstance().factories_;
    auto it = factories.find(k);
    if (it == factories.end()) {
      LOG(ERROR) << "CreateInstance - not exist func key: " << k;
      return false;
    } else {
      // TODO(@benqi): 是否需要抛出异常？
      (c->*it->second)(args...);
      return true;
    }
  }

  // 执行类成员函数
  template<typename... Args>
  static typename std::result_of<F(Args... args)>::type Execute3(const K& k, Args... args) {
    auto& factories = GetInstance().factories_;
    auto it = factories.find(k);
    if (it == factories.end()) {
      // TODO(@benqi): 是否需要抛出异常？
      LOG(ERROR) << "CreateInstance - not exist func key: " << k;
      // return false;
    } else {
      // return it->second(args...);
      // return true;
    }
    
    return it->second(args...);
  }

  // 检查是K否存在
  static bool Check(const K& k) {
    auto& factories = GetInstance().factories_;
    return factories.find(k) != factories.end();
  }
  
private:
  FuncFactoryManager() = default;
  FuncFactoryManager(const FuncFactoryManager&) = delete;
  FuncFactoryManager(FuncFactoryManager&&) = delete;
  
  static FuncFactoryManager& GetInstance() {
    // 有人说：
    //  在C++11里这个方法还是线程安全的，
    //  因为C++11中静态局部变量的初始化是线程安全的。
    // 真的？查查C++11的手册
    static FuncFactoryManager g_func_factorys;
    return g_func_factorys;
  }
  
  std::unordered_map<K, F> factories_;
};

// template<class F, class K>
// using FuncRegister = FuncFactoryManager<F, K>::RegisterTemplate;

}

#endif // NEBULA_BASE_FUNC_FACTORY_MANAGER_H_

