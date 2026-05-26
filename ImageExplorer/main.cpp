#include "pch.h"
#include "App.xaml.g.cpp"

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    winrt::Microsoft::UI::Xaml::Application::Start([](auto&&) {
        winrt::make<winrt::ImageExplorer::implementation::App>();
    });
    return 0;
}
