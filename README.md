# Raspberry Pi Zero 2 W · SH1106 OLED 控制面板

SH1106 128×64 I²C OLED (bus 1, address `0x3C`)。Astra 菜单使用两个高电平触发按钮：

| BCM GPIO（物理针脚） | 短按（松开触发） | 长按约 650ms |
| --- | --- | --- |
| GPIO24（Pin 18） | 上一项 | 确认 |
| GPIO23（Pin 16） | 下一项 | 取消 / 返回 |

按住期间，屏幕底部覆盖从 0% 增长到 100% 的进度条（OK/BACK）。长按达到阈值时操作只触发一次；松开不补发短按。使用 20ms 消抖、650ms 长按阈值（从消抖确认后计起）。GPIO 每 5ms 在独立线程采样，按键事件排队供 UI 消费，不受 OLED I²C 传输阻塞；UI 每帧检查事件，不再附加 50ms 轮询节流。OLED 使用 8×8 tile 差异传输，未改变内容时不重复刷新。短按在松开并消抖后生效，以免短按操作干扰同一个按钮的长按功能。按钮连接 **3.3V → 按钮 → GPIO**，GPIO 由 Linux 内核设置内部下拉，**禁止接 5V**。双键同时操作不作为独立组合键使用。

GPIO 使用 Linux GPIO character-device v2 (`/dev/gpiochip0`)，无需以 root 运行；OLED 通过 `/dev/i2c-1` 访问。运行账户需属于 `gpio` 和 `i2c` 用户组。启动时若 GPIO23/24 已被其他进程占用则会失败，避免重复驱动。

## ARM64 交叉编译（通过 SSHFS 使用目标系统 sysroot）

开发机需安装 `cmake`、`g++-aarch64-linux-gnu`、`gcc-aarch64-linux-gnu`、`fuse3`。脚本会在缺少 sshfs 时通过 `apt download sshfs` 自动下载并解压到项目的 `.tools/`，**无需 sudo**；SSH 需已配置公钥登录。目标机为 Debian 13 / Raspberry Pi OS Lite arm64。

```bash
# 挂载目标机根文件系统为只读 sysroot（不会向远端写入）
./scripts/mount-sysroot.sh

# 默认构建 SH1106、双按钮、系统状态、Wi-Fi/NetworkManager（必需功能）
./scripts/build-zero2w.sh

# 查看交叉编译 ELF 类型
file build-zero2w/raspberry_pi_oled_panel

# 如需复制程序到 Zero 2 W（不会自动运行）
scp build-zero2w/raspberry_pi_oled_panel wenyiyu@192.168.137.71:~/
```

可以通过 `PI_SSH=user@IP` 与 `PI_SYSROOT=/absolute/mount` 覆盖默认目标和挂载路径。挂载后手动卸载：`fusermount3 -u .sysroot`。

交叉编译工具链位于 `cmake/toolchains/zero2w-arm64.cmake`；所有 C/C++ 标准库和系统头文件从挂载的目标 sysroot 读取。注意目标系统必须安装对应 *-dev 开发包才能启用需要额外库的功能。

## 可选功能（GPIO12 风扇默认开启，其他传感器默认关闭）

| 构建开关 | 作用 |
| --- | --- |
| `PANEL_ENABLE_INA226` | INA226 电流/电压监测 |
| `PANEL_ENABLE_SGP30` | SGP30 空气质量 |
| `PANEL_ENABLE_SHT3X` | SHT3x 温湿度 |
| `PANEL_ENABLE_BMP280` | BMP280 气压/温度 |
| `PANEL_ENABLE_FAN` | GPIO12 CPU 温度控制风扇，默认 ON；可通过 OFF 完全排除 |
| `PANEL_ENABLE_RELAY` | 继电器 |
| `PANEL_ENABLE_SENSORS` | 开启其余可选传感器/继电器（风扇独立构建） |

| `PANEL_ENABLE_RPC` | 原项目 gRPC 库，默认关闭（当前主程序未使用） |

例如只开启 BMP280：

```bash
PANEL_ENABLE_BMP280=ON ./scripts/build-zero2w.sh
```

**网络功能始终启用**，不可通过 `PANEL_ENABLE_NETWORK=OFF` 关闭。包含 Wi-Fi/NetworkManager 管理界面和系统网络状态。目标 sysroot 必须安装 `libnm-dev` 及其依赖（GLib/GIO 开发头），开发机需有 `pkg-config`。构建脚本可在缺少该命令时使用项目 `.tools/pkgconfig/` 内的本地副本。

## 本机输入逻辑单元测试

```bash
g++ -std=c++17 -I. tests/two_button_input_test.cpp -o /tmp/two_button_test
/tmp/two_button_test
```

不创建任何开机自启服务；程序运行与部署均为手动操作。

### Wi-Fi 扫描崩溃诊断

`wifi` 扫描只发起一次请求，完成后复用 NetworkManager 缓存的热点列表。UI 线程在后台扫描结束后统一重建菜单，避免后台线程删除正在使用的菜单。每次重建菜单都会重置选中索引。

在 Raspberry Pi 上可运行不占用 GPIO/I²C 的扫描诊断：

```bash
~/raspberry_pi_oled_panel --test-wifi-scan wlan0 5
```

此命令会循环扫描 5 次并显示每次获取的热点数量和耗时，不会启动 OLED 界面或更改 Wi-Fi 连接。

### 系统状态页紧凑布局

系统状态菜单采用独立的 `listItemGap=2px`（Astra 其他列表保留默认 4px）；网络名称和 IP 两行间距仍为 1px。选中背景框额外下移 1px，同时相应缩减高度，避免贴着文字底边和覆盖下一行。相机滚动边界继续按照列表真实行距计算。128×64 屏幕上可更紧凑地显示 IP、CPU、内存、磁盘及网速状态，无须降低字体可读性。

## 电源管理弹窗

主界面按住 GPIO23（取消/返回）约 650ms，弹出电源管理窗口；在二级页面长按仍然返回上级。弹窗仅提供“关机 / 重启”，默认选中“关机”。短按 GPIO24/23 在两项间切换，长按 GPIO24 确认，长按 GPIO23 直接关闭弹窗（无需单独的“取消”选项）。选中关机或重启后还有第二次“执行 / 返回”确认，默认停留“返回”以防误触。

面板按照需求**整体以 root 身份运行**，电源动作直接调用 `/usr/bin/systemctl --no-block poweroff|reboot`，无需经 sudoers 运行。请在目标机通过以下脚本手动管理面板（**不会开机自启**）：

```bash
# 首次启动会提示输入 wenyiyu 的 sudo 密码（如未配置免密）
/home/wenyiyu/run-panel-root.sh start
# 停止或前台调试
/home/wenyiyu/run-panel-root.sh stop
/home/wenyiyu/run-panel-root.sh foreground
# 检查 root 进程
ps -eo pid,user,args | grep '[r]aspberry_pi_oled_panel'
```

`start` 会先终止旧的普通用户实例，避免 GPIO23/24 和 I²C 占用冲突。root 进程日志保存到 `/var/log/raspberry-pi-oled-panel.log`。该方案扩大面板进程的系统权限，请确保只运行受信任的构建产物。

仅测试弹窗的选择与取消逻辑（不会执行系统电源操作）：

```bash
g++ -std=c++17 -I. tests/power_dialog_state_test.cpp -o /tmp/power_dialog_state_test
/tmp/power_dialog_state_test
```


## GPIO12 风扇温度控制

GPIO12（BCM12，**物理针脚 32**）作为风扇驱动控制输出，使用 Linux GPIO v2 字符设备。**请使用外接 MOSFET/三极管驱动及续流保护，不要从 GPIO 引脚直接给风扇供电。** 默认高电平开启；如果实际驱动电路低电平开启，可在 OLED 设置中改变有效电平。此版本提供温度阈值开关控制，不输出 PWM 调速波形。

`PANEL_ENABLE_FAN=ON` 为 Zero 2 W 默认构建选项，无需开启 INA226、SGP30、BMP280、SHT3x、继电器等其他模块。`PANEL_ENABLE_FAN=OFF` 时不编译控制器，也不申请 GPIO12；NetworkManager 仍强制启用。

从 OLED 主屏进入 **设置 → 风扇温控**：

- **模式**：自动 → 常开 → 关闭。长按 GPIO24 切换模式。
- **启动温度**：默认 **55℃**；长按 GPIO24 进入编辑，短按 GPIO24 +1℃、GPIO23 -1℃，长按 GPIO24 保存、GPIO23 取消。
- **停止温度**：默认 **45℃**；操作同上。始终要求启动温度比停止温度高至少 2℃，防止频繁开关。
- **有效电平**：默认为 **高有效**，可切换为低有效，以适配不同风扇驱动板。
- **状态**：显示实时 CPU 温度和风扇 GPIO 控制状态（无转速反馈）；每秒更新。

自动模式下，CPU 温度 ≥ 启动阈值时开启风扇，≤ 停止阈值时关闭；两者之间保持之前状态，避免反复启停。如果温度传感器读取失败，自动模式会**强制开启风扇（故障安全）**。常开或关闭模式遵从手动选择。

配置自动保存到目标机 `/var/lib/raspberry-pi-oled-panel/fan.conf`，重启面板会加载上次保存的参数。温度控制在独立线程中执行，不受 OLED I²C 绘图或 Wi-Fi 扫描影响。面板仍以 root 手动启动，不创建新的开机自启服务；**面板未运行期间不提供 GPIO12 温控**。

单元测试：

```bash
g++ -std=c++17 -I. tests/fan_policy_test.cpp -o /tmp/fan_policy_test
/tmp/fan_policy_test
```


## 网卡启用/禁用确认弹窗

在 **网络接口** 页面选中网卡后，长按 GPIO24 不再立即更改网卡状态，而是打开确认弹窗，显示网卡名及“确认启用网卡?” / “确认禁用网卡?”。弹窗默认高亮 **取消**，短按 GPIO24 / GPIO23 在 **执行 / 取消** 之间切换；只有选中 **执行** 并再次长按 GPIO24 才会调用 `SIOCSIFFLAGS` 修改网卡状态。长按 GPIO23 可随时取消，不修改网卡或开关显示。执行失败时恢复之前的开关指示。

**注意：** 禁用当前正在承载 SSH 的网卡会使远程连接中断；仅通过屏幕实体按钮确认，部署和功能检查不会主动禁用任何实际网卡。

```bash
g++ -std=c++17 -I. tests/network_confirmation_state_test.cpp -o /tmp/network_confirmation_test
/tmp/network_confirmation_test
```


## 系统状态 IP 显示与屏幕方向

系统状态页的网络部分只显示 **IP 地址**（不再显示 wlan0/eth0 等网卡名称），网卡名仍在内部用于网络速度统计。没有可用地址时显示“无IP”。状态信息保持紧凑列表行距。

通过 **设置 → 屏幕方向**，选择 **0度 / 180度** 两项，当前方向带 `[当前]` 标记。长按确认后立即生效，不必重启面板。SH1106 的 180° 方向采用硬件扫描翻转，包含所有菜单、弹窗和长按进度条。

选择 180° 时，**GPIO23 和 GPIO24 的全部操作同时对调**：GPIO23 短按上一项、长按确认；GPIO24 短按下一项、长按返回。切回 0° 即恢复 GPIO24 短按上一项/长按确认、GPIO23 短按下一项/长按返回。长按进度条的 OK/BACK 标签也与新按键定义保持一致。

方向只能设为 0 或 180，默认 0。每次变更原子保存到目标机的 `/var/lib/raspberry-pi-oled-panel/display.conf`，重启后自动加载；文件异常时采用 0°。可通过修改为 `0` 并重启面板恢复默认方向。

方向映射单元测试：

```bash
g++ -std=c++17 -I. tests/display_orientation_test.cpp -o /tmp/display_orientation_test
/tmp/display_orientation_test
```


## 系统状态页 30 秒静置显示

进入 **系统状态** 页后，如果连续 **30 秒** 无按键操作，自动隐藏选中背景框和右侧滚动条，仅保留 IP、CPU、内存、磁盘、网络速度等实时状态文字；数据仍继续刷新。短按或长按任意按钮均恢复选中框和滚动条，并正常执行按键操作，同时重新计时。退出并重新进入状态页也会重新计时。其他菜单不受影响。

逻辑单元测试：`g++ -std=c++17 -I3rdparty/astra/include tests/navigation_idle_timer_test.cpp -o /tmp/navigation_idle_timer_test && /tmp/navigation_idle_timer_test`。


### 配置系统状态页的自动隐藏时间

在 **设置 → 状态隐藏** 可修改系统状态页面无按键后自动隐藏选中背景框、右侧滚动条的时间。默认为 **30 秒**，取值 **5～300 秒**，每次按上一项增加 5 秒、下一项减少 5 秒；设为 **0 秒（关闭）** 时不再自动隐藏。长按确认键保存，长按返回键取消（按钮在 180° 屏幕方向下仍按对应逻辑交换）。设置保存至 `/var/lib/raspberry-pi-oled-panel/status-idle.conf`，下次进入系统状态页面立即使用新时间，并在设备重启后保持不变。数据的实时刷新不受该设置影响。
