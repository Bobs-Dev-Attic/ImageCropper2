#pragma once
#include "MainWindow.g.h"
#include "ImageItem.h"
#include <vector>

namespace winrt::ImageExplorer::implementation {
struct MainWindow : MainWindowT<MainWindow> {
    MainWindow();
    fire_and_forget OpenFolder_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    fire_and_forget OpenFolderMenu_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    fire_and_forget OpenFile_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    void Preferences_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    void Exit_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    void AnalyzeImage_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    void DetectPeople_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    void DetectVehicles_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    void DetectAnimals_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    void DetectTextRegions_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    void ThumbSizeSmall_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    void ThumbSizeMedium_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    void ThumbSizeLarge_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    void ThumbSizeSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const&);
    void ImageGrid_ItemClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Controls::ItemClickEventArgs const&);
    void ImageScrollViewer_PointerWheelChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&);
    void ImageScrollViewer_PointerPressed(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&);
    void ImageScrollViewer_PointerMoved(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&);
    void ImageScrollViewer_PointerReleased(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&);
    void OverlayCanvas_PointerPressed(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&);
    void OverlayCanvas_PointerMoved(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&);
    void OverlayCanvas_PointerReleased(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&);
    void OverlayCanvas_RightTapped(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const&);
private:
    winrt::Windows::Foundation::Collections::IObservableVector<winrt::Windows::Foundation::IInspectable> m_items;
    std::vector<winrt::Windows::Storage::StorageFile> m_files;
    winrt::Windows::Foundation::IAsyncAction PickAndLoadFolderAsync();
    winrt::Windows::Foundation::IAsyncAction LoadFolderAsync(winrt::Windows::Storage::StorageFolder const& folder);
    winrt::Windows::Foundation::IAsyncOperation<winrt::Microsoft::UI::Xaml::Media::Imaging::ImageSource> CreateThumbnailAsync(winrt::Windows::Storage::StorageFile const& file);
    fire_and_forget ShowImageAsync(winrt::Windows::Storage::StorageFile const& file);
    void RefreshThumbnailSizes();
    void SetThumbnailSize(double size);
    winrt::Windows::Foundation::IAsyncAction PopulateMetadataAsync(winrt::Windows::Storage::StorageFile const& file);
    void AddDetectionBox(float x, float y, float w, float h);
    void ShowBoxMenu(float x, float y);
    bool m_isPanning{ false }, m_isDrawing{ false };
    winrt::Windows::Foundation::Point m_lastPoint{}, m_startPoint{};
    winrt::Microsoft::UI::Xaml::Shapes::Rectangle m_currentRect{ nullptr };
    double m_zoom{ 1.0 };
};
}
namespace winrt::ImageExplorer::factory_implementation { struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow> {}; }
