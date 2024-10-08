/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * kiran-session-manager is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <QObject>

namespace Kiran
{
namespace WAAM
{
class WindowManager;

class AppManager : public QObject
{
    Q_OBJECT

public:
    static AppManager *getInstance()
    {
        return m_instance;
    };
    static void globalInit(WindowManager *windowManager);
    static void globalDeinit();

    //     // 从对应的uri创建用户自定义应用的desktop文件，并返回用户自定义应用的desktop_id
    //     std::string create_userapp_from_uri(const std::string &uri);

    //     bool remove_app_from_disk(const std::string &desktop_id);

    //     // 获取所有App列表(每个App对应一个desktop文件)
    //     AppVec get_apps();

    //     // 获取制定类型的App列表
    //     AppVec get_apps_by_kind(AppKind kind);

    //     // 获取可以在菜单中显示的App列表
    //     AppVec get_should_show_apps();

    //     // 获取正在运行的App列表
    //     AppVec get_running_apps();

    //     // 通过desktop_id获取App
    //     std::shared_ptr<App> lookup_app(const std::string &desktop_id);

    //     // 通过窗口对象获取App
    //     std::shared_ptr<App> lookup_app_with_window(std::shared_ptr<Window> window);

    //     // 通过WnckApplication的xid获取App
    //     std::shared_ptr<App> lookup_app_with_xid(uint64_t xid);

    //     // 获取所有App的desktop_id，并根据desktop文件的Name字段进行排序
    //     std::vector<std::string> get_all_sorted_apps();

    //     // desktop应用列表发生变化信号
    //     sigc::signal<void> &signal_desktop_app_changed() { return this->app_desktop_changed_; }
    //     // App安装时的信号
    //     sigc::signal<void, AppVec> &signal_app_installed() { return this->app_installed_; }
    //     // App卸载时的信号
    //     sigc::signal<void, AppVec> &signal_app_uninstalled() { return this->app_uninstalled_; }
    //     // 应用程序状态发生变化
    //     sigc::signal<void, std::shared_ptr<App>, AppAction> &signal_app_action_changed() { return this->signal_app_action_changed_; }

    // private:
    //     AppManager(WindowManager *window_manager);
    //     virtual ~AppManager(){};

    //     void init();

    // private:
    //     std::shared_ptr<App> get_app_from_sandboxed_app(std::shared_ptr<Window> window);
    //     std::shared_ptr<App> get_app_from_gapplication_id(std::shared_ptr<Window> window);
    //     std::shared_ptr<App> get_app_from_cmdline(std::shared_ptr<Window> window);
    //     std::shared_ptr<App> get_app_from_window_wmclass(std::shared_ptr<Window> window);
    //     std::shared_ptr<App> lookup_app_with_wmclass(const std::string &wmclass);
    //     std::shared_ptr<App> lookup_app_with_unrefine_name(const std::string &name);
    //     std::shared_ptr<App> lookup_app_with_heuristic_name(const std::string &name);
    //     std::shared_ptr<App> get_app_from_env(std::shared_ptr<Window> window);
    //     std::shared_ptr<App> get_app_by_enumeration_apps(std::shared_ptr<Window> window);
    //     std::shared_ptr<App> get_app_by_enumeration_windows(std::shared_ptr<Window> window);
    //     std::shared_ptr<App> get_app_from_window_group(std::shared_ptr<Window> window);

    //     std::string gen_userapp_id(const std::string &userapp_dir, const std::string &desktop_id);
    //     std::string get_userapp_dir_path();

    //     void load_desktop_apps();
    //     void clear_desktop_apps();
    //     void register_app(std::map<std::string, std::shared_ptr<App>> &old_apps,
    //                       Glib::RefPtr<Gio::AppInfo> &app,
    //                       AppKind kind);

    //     void register_app(std::map<std::string, std::shared_ptr<App>> &old_apps,
    //                       const std::string &desktop_file,
    //                       AppKind kind);

    //     // desktop应用变化的信号处理
    //     static void desktop_app_changed(AppManager *manager);

    //     // 启动一个应用时的信号处理
    //     static void app_opened(WnckScreen *screen, WnckApplication *wnck_application, gpointer user_data);
    //     // 关闭一个应用时的信号处理
    //     static void app_closed(WnckScreen *screen, WnckApplication *wnck_application, gpointer user_data);

    //     // 处理窗口打开信号
    //     void window_opened(std::shared_ptr<Window> window);
    //     // 处理窗口关闭信号
    //     void window_closed(std::shared_ptr<Window> window);

    //     // 更新WnckApplication和App之间的绑定关系
    //     void update_wnckapps_binding();
    //     // 绑定WnckApplication到App，返回绑定成功的App和App是否为新创建的
    //     std::pair<std::shared_ptr<App>, bool> bind_wnck2app(WnckApplication *wnck_application);
    //     // 绑定WnckApplication到FakeApp，返回绑定成功的FakeApp
    //     std::shared_ptr<App> bind_wnck2fake(WnckApplication *wnck_application);
    //     // 取消WnckApplication和App的绑定关系，返回取消绑定的App和App是否被删除
    //     std::pair<std::shared_ptr<App>, bool> unbind_wnck2app(WnckApplication *wnck_application);
    //     // 清理所有WnckApplication和App之间的绑定关系
    //     void clear_wnckapps_binding();
    //     // 清理未使用的fake app，返回清理的数量
    //     int32_t clear_nouse_fake_app();

    //     std::string get_exec_name(const std::string &exec_str);

    //     void app_launched(std::shared_ptr<App> app);
    //     void app_close_all_windows(std::shared_ptr<App> app);

    // protected:
    //     sigc::signal<void> app_desktop_changed_;
    //     sigc::signal<void, AppVec> app_installed_;
    //     sigc::signal<void, AppVec> app_uninstalled_;
    //     sigc::signal<void, std::shared_ptr<App>, AppAction> signal_app_action_changed_;

private:
    static AppManager *m_instance;
    WindowManager *m_windowManager;

    // std::map<std::string, std::shared_ptr<App>> apps_;
    // std::map<std::string, std::weak_ptr<App>> wmclass_apps_;

    // // <WnckApplication xid, App>
    // std::map<uint64_t, std::weak_ptr<App>> wnck_apps_;

    // GAppInfoMonitor *system_app_monitor; /* 系统应用监控器 */
    // GFileMonitor *user_app_monitor;      /* 用户应用目录监控器 */
};

}  // namespace WAAM

}  // namespace Kiran