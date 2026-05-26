#pragma once
#include "MainWindow.g.h"
#include "ImageItem.h"
#include <vector>

namespace winrt::ImageExplorer::implementation {
struct MainWindow : MainWindowT<MainWindow> {
    MainWindow();
    fire_and_forget OpenFolder_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    void ThumbSizeSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const&);
private:
    winrt::Windows::Foundation::Collections::IObservableVector<winrt::Windows::Foundation::IInspectable> m_items;
    winrt::Windows::Foundation::IAsyncAction LoadFolderAsync(winrt::Windows::Storage::StorageFolder const& folder);
    winrt::Windows::Foundation::IAsyncOperation<winrt::Microsoft::UI::Xaml::Media::Imaging::ImageSource> CreateThumbnailAsync(winrt::Windows::Storage::StorageFile const& file);
};
}
namespace winrt::ImageExplorer::factory_implementation { struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow> {}; }
