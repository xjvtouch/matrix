/*
 * Tencent is pleased to support the open source community by making wechat-matrix available.
 * Copyright (C) 2018 THL A29 Limited, a Tencent company. All rights reserved.
 * Licensed under the BSD 3-Clause License (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android/log.h>
#include <thread>
#include "fd_info_collector.h"
#include "comm/fd_canary_utils.h"


namespace fdcanary {

    void FDInfoCollector::OnOpen(std::string &key, std::string &stack) {

        info_map_.insert(std::make_pair(key, stack));
        __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "FDInfoCollector::OnOpen size is[%zu],key is [%s]]",
            info_map_.size(), key.c_str());
        
    }


    void FDInfoCollector::OnClose(std::string &key) {
        __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "FDInfoCollector::OnClose size is [%zu], key:[%s]",
            info_map_.size(), key.c_str());
        std::unordered_map<std::string, std::string>::iterator it;
        it = info_map_.find(key);
        if (it != info_map_.end()) {
            info_map_.erase(key);
           __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI","FDInfoCollector::OnClose erase success value is %s", it->second.c_str());
        } else {
           __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI","FDInfoCollector::OnClose erase fail");
        }
    }
}
