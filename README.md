# Overview

![avatar]()

## 框架简介

* simple.flow支持自动生成对应代码并且完成相对应的模块注册[低代码平台]
* simple.flow支持快速的定义产品的流程，并且快速完成部署[DAG]

## 设计目的

* 提高算法原型(pyhton)到产品部署的效率
* 减少代码冗余，提高算法可维护性，可拓展性
* simple.flow能够完全流水线处理，高效的完成整个流程的处理

## 环境配置

* 系统：Ubuntu 16.04, GCC 5+
* [必须依赖]
  1. simple.base 基础框架(https://github.com/jiangyanpeng/simple.base) 
* [可选依赖]
  1. simple.cv 图像处理加速框架(https://github.com/jiangyanpeng/simple.cv) 
  2. gooletest 编译单元测试 (https://github.com/google/googletest.git)
  3. spdlog 日志管理 (https://github.com/gabime/spdlog.git)
  4. simple.nn 推理框架(https://github.com/jiangyanpeng/simple.nn) [doing]
  5. simple.llm 大模型推理框架(https://github.com/jiangyanpeng/simple.llm) [todo]


## 编译

* 可以参考使用scripts/build_linux_x86.sh的脚本(build && install)
* 编译选项
  1. 


## [代码规范]
* 分支管理
  * 在新建分支开发时，尽量基于最新的开发分支进行，减少后续的合并冲突
  * 命名规范:<type>/+动宾描述 (example: fix/fix_detector_memory_leak)
* git message规范，请按照：`<type>(<scope>): <subject>` 格式编写提交信息，其中每个字段含义为：
* `<type>`：提交的类型，必填！只能为以下七种类型中的一种：
  * feat：新功能（面向用户的新功能特性，或有重大提升的功能）
  * fix：问题修复（侧重于功能的修复，而非编译、构建或部署时的错误）
  * test：测试相关
  * refactor：重构代码（除新功能和问题修复以外的代码变动）
  * docs：文档相关
  * style：代码风格相关
  * chore：其他（以上六种均不符合时，归到本类型中，比如构建、部署相关）
* `<subject>`：提交的主题信息，**必填**.
* `<scope>`：提交涉及的范围，可选（不填时要连同括号一起省略）

reference: 