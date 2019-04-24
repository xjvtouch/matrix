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
#include <fcntl.h>
#include <sys/stat.h>
#include "fd_info_collector.h"
#include "comm/fd_canary_utils.h"
#include "core/fd_dump.h"


namespace fdcanary {

    void FDInfoCollector::OnOpen(int fd, std::string &stack) {
        __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "FDInfoCollector::OnOpen fd:[%d]", fd);
        int type = GetType(fd);
        if (type != -1) {
            InsertTypeMap(type, fd, stack);
        }
    }


    void FDInfoCollector::OnClose(int fd) {
        __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "FDInfoCollector::OnClose fd:[%d]", fd);
        int type = GetType(fd);
        if (type != -1) {
            RemoveTypeMap(type, fd);
        }
    }

    int FDInfoCollector::GetType(int fd) {
        int type;
        int flags = fcntl(fd, F_GETFD, 0);
        if (flags != -1) {
            struct stat statbuf;
            if (fstat(fd, &statbuf) == 0) {
                type = (S_IFMT & statbuf.st_mode);
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI","FDInfoCollector::GetType type is [%d]", type);
                return type;
            }
        } else {
            __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI","FDInfoCollector::GetType flags == -1");
        }
        return -1;
    }

    void FDInfoCollector::InsertTypeMap(int type, int fd, std::string &stack) {
        switch (type) {
            case S_IFIFO:
                //命名管道
                pipe_map_.insert(std::make_pair(fd, stack));
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "FDInfoCollector::InsertTypeMap (named pipe) | fd is [%d]]", fd);
                break;
            case S_IFCHR:
                // 字符设备（串行端口）
                dmabuf_map_.insert(std::make_pair(fd, stack));
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "FDInfoCollector::InsertTypeMap (character special) | fd is [%d]]", fd);
                break;
            case S_IFREG:
                //普通文件
                io_map_.insert(std::make_pair(fd, stack));
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "FDInfoCollector::InsertTypeMap (regular) | fd is [%d]]", fd);    
                break;
            case S_IFSOCK:
                //socket 
                socket_map_.insert(std::make_pair(fd, stack));
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "FDInfoCollector::InsertTypeMap (socket) | fd is [%d]]", fd);
                break;
                
            case S_IFDIR:
                //目录
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "directory");
                break;
            case S_IFBLK:
                //块设备（数据存储接口设备）
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "block special");
                break;
            case S_IFLNK:
                //符号链接文件（文件的软连接文件，类似于window的快捷方式）
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "symbolic link");
                break;
            
            default:
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "<unknown>");
                break;
        }
    }

    void FDInfoCollector::RemoveTypeMap(int type, int fd) {
        switch (type) {
            case S_IFIFO:
                //命名管道
                RemoveImpl(fd, pipe_map_);
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "FDInfoCollector::RemoveTypeMap (named pipe) | fd is [%d]]", fd);
                break;
            case S_IFCHR:
                // 字符设备（串行端口）
                RemoveImpl(fd, dmabuf_map_);
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "FDInfoCollector::RemoveTypeMap (character special) | fd is [%d]]", fd);
                break;
            case S_IFREG:
                //普通文件
                RemoveImpl(fd, io_map_);
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "FDInfoCollector::RemoveTypeMap (regular) | fd is [%d]]", fd); 
                break;   
            case S_IFSOCK:
                //socket 
                RemoveImpl(fd, socket_map_);
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "FDInfoCollector::RemoveTypeMap (socket) | fd is [%d]]", fd);
                break;

                
            case S_IFDIR:
                //目录
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "directory");
                break;
            case S_IFBLK:
                //块设备（数据存储接口设备）
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "block special");
                break;
            case S_IFLNK:
                //符号链接文件（文件的软连接文件，类似于window的快捷方式）
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "symbolic link");
                break;
            
            default:
                __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "<unknown>");
                break;
        }
    }

    void FDInfoCollector::RemoveImpl(int fd, std::unordered_map<int, std::string> &map) {
        std::unordered_map<int, std::string>::iterator it;
        it = map.find(fd);
        if (it != map.end()) {
            map.erase(fd);
           __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI","FDInfoCollector::RemoveImpl erase success, fd is [%d], current size is [%zu], value is [%s]", fd, map.size(), it->second.c_str());
        } else {
           __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI","FDInfoCollector::RemoveImpl erase fail, fd is [%d], current size is [%zu]", fd, map.size());
        }
    }
}
