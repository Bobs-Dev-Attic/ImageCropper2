#pragma once
#include "App.xaml.g.h"

namespace winrt::ImageExplorer::implementation {
struct App : AppT<App> {
    App();
    void OnLaunched(winrt::Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
private:
    winrt::Microsoft::UI::Xaml::Window m_window{ nullptr };
};
}
namespace winrt::ImageExplorer::factory_implementation { struct App : AppT<App, implementation::App> {}; }
