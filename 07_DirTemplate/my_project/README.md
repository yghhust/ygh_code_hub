# 1 概述

本规范参考业界经验，基于模型与代码分离、应用/业务和基础设施分层组织、模块化设计、第三方库隔离等原则制定。

# 2 目录概览

```
xxx_project
├── model/
│   ├── model/                 # 状态机建模
│   └── Coco.toml
├── conifg/                    # 全局配置
├── include/                   # 全局公共头文件
├── src/                       # 全局基础设施实现
├── modules/                   # 功能模块
│   ├── module_a/
│   │   ├── include/           # 模块公共头
│   │   └── src/               # 模块私有实现
│   └── module_b/
├── apps/                      # 应用程序
│   ├── cli/
│   │   ├── include/           # 命令行应用
│   │   └── src/               # 应用实现
│   └── gui/                   # 图形界面应用
├── tests/                     # 测试
├── third_party/               # 第三方依赖
├── CMakeLists.txt             # CMake模块
└── README.md
```

# 3 目录依赖关系
```
                    apps/
                      ↑
    ------------------------------------
    |                 |                |
modules/          model_gen/      tests/ (集成测试)
    ↑                 ↑
    |                 |
    -------------------
                ↓
         src/ (基础设施)
                ↓
        third_party/
```

# 4 本目录规范遵循的关键原则

|原则|说明	|在本结构中的体现
|:----|:-----|:-----
模块化|功能按模块划分，模块间低耦合、高内聚	|modules/ 下每个独立模块
接口与实现分离|	头文件暴露接口，源文件隐藏实现细节	|模块内 include/ 和 src/ 分离
分层架构|	全局基础设施、模块、应用自上而下依赖	|include/src → modules → apps
可测试性|	测试代码独立，可针对模块或整体测试	|tests/ 独立目录
第三方依赖隔离|	外部库与项目代码物理分离	|third_party/
模型与代码分离|	领域模型独立维护，支持代码生成	|model/ 目录
构建集成|	顶层 CMakeLists 统一管理，支持模块化构建	|根目录 CMakeLists.txt
可扩展性|	新增模块或应用只需添加对应目录，不影响现有结构	|modules/ 和 apps/ 可水平扩展

---
# 5 贡献

欢迎提交 Issue 和 Pull Request！

# 6 许可证

本项目采用 [MIT License](LICENSE) 开源许可证。
