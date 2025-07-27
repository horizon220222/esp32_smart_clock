# ESP32-WiFi-Sync Mechanical Segment Clock

一个基于 ESP32-S3（兼容 ESP32）的 Wi-Fi 同步机械段码时钟。  
核心功能：
- 通过 NTP 自动同步时间（支持时区）。
- 舵机驱动 88:88 机械段码显示当前时间。
- 支持最多 10 组闹钟/定时器。
- 模块化 C 语言代码，方便后期扩展语音等子系统。

## 1. 仓库结构

```angular2html
├── CMakeLists.txt          # ESP-IDF 根构建
├── README.md               # 本文档
├── sdkconfig.defaults      # 默认配置
├── partitions.csv          # 分区表（应用 + NVS 足够）
├── main/
│   ├── wifi_time/          # Wi-Fi + SNTP 时间同步
│   ├── servo_display/      # 舵机驱动 88:88 机械段码
│   ├── alarm_mgr/          # 闹钟/定时任务管理
│   └── utils/              # 通用工具（json、queue 封装）
│   ├── main.c              # 入口：初始化 + 超级循环
│   └── app_state.h/.c      # 全局状态机
└── test/                   # Unity 单元测试
```

## 2. 运行时架构

```angular2html
┌────────────┐
│   main.c   │ 超级循环 + 事件队列
└────┬───────┘
     │ esp_event
┌────┴───────┐     ┌────────────────┐
│ app_state  │<--->│   alarm_mgr    │
└────┬───────┘     └────────────────┘
     │
┌────┴───────┐     ┌────────────────┐
│ wifi_time  │<--->│ servo_display  │
└────────────┘     └────────────────┘
```

## 3. 模块职责

| 模块          | 任务与接口                                                                                                          |
|---------------|----------------------------------------------------------------------------------------------------------------|
| **wifi_time** | 1. 启动 STA 模式，连接已知 SSID<br>2. SNTP 获取 UTC，支持 POSIX 时区字符串<br>3. 每分钟发布 `TIME_SYNC_EVT`                            |
| **servo_display** | 1. 初始化 2 * 16 路 PWM（LEDC）驱动舵机阵列<br>2. 提供 `display_set(const char* str)` 接口，支持 "88:88"<br>3. 平滑动画，避免抖动          |
| **alarm_mgr** | 1. NVS 持久化闹钟列表（最多 10 组）<br>2. 提供 `alarm_add()` / `alarm_del()` / `alarm_list()`<br>3. 匹配触发后发布 `ALARM_FIRE_EVT` |
| **utils**     | 1. 轻量 cJSON 封装，方便后期 Web 配网<br>2. 线程安全队列包装 `xQueue`                                                             |
| **main.c**    | 1. 初始化 NVS、事件循环、各组件<br>2. 等待 `TIME_SYNC_EVT` 后启动 1 Hz 软定时器刷新显示<br>3. 收到 `ALARM_FIRE_EVT` 启动蜂鸣器/LED             |

---

## 4. 配置与编译

```bash
idf.py set-target esp32    # 或 esp32
idf.py menuconfig          # 在 Example Configuration 中填写 Wi-Fi SSID/PWD
idf.py flash monitor
```

## 5. 注意事项

- 舵机供电与 ESP32 逻辑供电隔离，避免复位。  
- 使用 LEDC 2 * 16 路 PWM，频率 50 Hz，占空比 500-2500 µs。
- 默认时区 CST-8，在 menuconfig 里可改。  