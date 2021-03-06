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

#include "nebula/base/config_util.h"

#include <iostream>
#include <folly/Format.h>

#include "nebula/base/configuration.h"

namespace nebula {
  
bool SystemConfig::SetConf(const std::string& conf_name, const Configuration& conf) {
    folly::dynamic v = nullptr;
    
    v = conf.GetValue("io_thread_pool_size");
    if (v.isInt()) io_thread_pool_size = static_cast<uint32_t>(v.asInt());

    return true;
}

}
