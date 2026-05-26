#pragma once
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.Storage.h>

struct ImageItem {
    winrt::hstring Name;
    winrt::Windows::Storage::StorageFile File{ nullptr };
    winrt::Microsoft::UI::Xaml::Media::Imaging::ImageSource Thumbnail{ nullptr };
};
