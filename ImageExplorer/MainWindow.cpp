#include "pch.h"
#include "MainWindow.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.FileProperties.h>
#include <winrt/Windows.System.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media::Imaging;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::FileProperties;

namespace winrt::ImageExplorer::implementation {
MainWindow::MainWindow() {
    InitializeComponent();
    m_items = single_threaded_observable_vector<IInspectable>();
    ImageGrid().ItemsSource(m_items);
}

fire_and_forget MainWindow::OpenFolder_Click(IInspectable const&, RoutedEventArgs const&) {
    auto lifetime = get_strong();
    FolderPicker picker;
    picker.FileTypeFilter().Append(L"*");
    auto hwnd = GetActiveWindow();
    auto init = picker.as<IInitializeWithWindow>();
    init->Initialize(hwnd);
    StorageFolder folder = co_await picker.PickSingleFolderAsync();
    if (folder) { co_await LoadFolderAsync(folder); }
}

IAsyncAction MainWindow::LoadFolderAsync(StorageFolder const& folder) {
    m_items.Clear();
    auto files = co_await folder.GetFilesAsync();
    for (auto const& file : files) {
        auto ext = to_lower(file.FileType().c_str());
        if (ext == L".png" || ext == L".jpg" || ext == L".jpeg" || ext == L".gif" || ext == L".svg") {
            auto panel = StackPanel();
            panel.Orientation(Orientation::Vertical);
            auto img = Image();
            img.Width(ThumbSizeSlider().Value());
            img.Height(ThumbSizeSlider().Value());
            img.Stretch(Microsoft::UI::Xaml::Media::Stretch::UniformToFill);
            img.Source(co_await CreateThumbnailAsync(file));
            auto txt = TextBlock();
            txt.Text(file.Name());
            txt.TextWrapping(TextWrapping::WrapWholeWords);
            txt.MaxWidth(ThumbSizeSlider().Value());
            panel.Children().Append(img);
            panel.Children().Append(txt);
            m_items.Append(panel);
        }
    }
}

IAsyncOperation<ImageSource> MainWindow::CreateThumbnailAsync(StorageFile const& file) {
    auto ext = to_lower(file.FileType().c_str());
    if (ext == L".svg") {
        auto source = SvgImageSource();
        auto stream = co_await file.OpenAsync(FileAccessMode::Read);
        co_await source.SetSourceAsync(stream);
        co_return source;
    }
    StorageItemThumbnail thumb = co_await file.GetThumbnailAsync(ThumbnailMode::PicturesView, static_cast<uint32_t>(ThumbSizeSlider().Value()), ThumbnailOptions::UseCurrentScale);
    auto bitmap = BitmapImage();
    co_await bitmap.SetSourceAsync(thumb);
    co_return bitmap;
}

void MainWindow::ThumbSizeSlider_ValueChanged(IInspectable const&, Controls::Primitives::RangeBaseValueChangedEventArgs const&) {
    if (FolderNameText().Text().empty()) { return; }
}
}
