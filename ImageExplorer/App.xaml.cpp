#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.h"
#if __has_include("App.xaml.g.cpp")
#include "App.xaml.g.cpp"
#endif

namespace winrt::ImageExplorer::implementation {
App::App() { InitializeComponent(); }
void App::OnLaunched(winrt::Microsoft::UI::Xaml::LaunchActivatedEventArgs const&) {
    m_window = winrt::make<MainWindow>();
    m_window.Activate();
}
}
