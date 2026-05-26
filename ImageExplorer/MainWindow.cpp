#include "pch.h"
#include "MainWindow.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.FileProperties.h>
#include <winrt/Windows.Globalization.DateTimeFormatting.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media;
using namespace Microsoft::UI::Xaml::Media::Imaging;
using namespace Microsoft::UI::Xaml::Shapes;
using namespace Microsoft::UI::Xaml::Input;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::FileProperties;

namespace winrt::ImageExplorer::implementation {
MainWindow::MainWindow() {
    InitializeComponent();
    m_items = single_threaded_observable_vector<IInspectable>();
    ImageGrid().ItemsSource(m_items);
}

fire_and_forget MainWindow::OpenFolder_Click(IInspectable const&, RoutedEventArgs const&) { co_await PickAndLoadFolderAsync(); }
fire_and_forget MainWindow::OpenFolderMenu_Click(IInspectable const&, RoutedEventArgs const&) { co_await PickAndLoadFolderAsync(); }

IAsyncAction MainWindow::PickAndLoadFolderAsync() {
    FolderPicker picker; picker.FileTypeFilter().Append(L"*");
    auto init = picker.as<IInitializeWithWindow>(); init->Initialize(GetActiveWindow());
    StorageFolder folder = co_await picker.PickSingleFolderAsync();
    if (folder) { FolderNameText().Text(folder.Name()); co_await LoadFolderAsync(folder); }
}

fire_and_forget MainWindow::OpenFile_Click(IInspectable const&, RoutedEventArgs const&) {
    FileOpenPicker picker; picker.FileTypeFilter().Append(L".png"); picker.FileTypeFilter().Append(L".jpg"); picker.FileTypeFilter().Append(L".jpeg"); picker.FileTypeFilter().Append(L".gif"); picker.FileTypeFilter().Append(L".svg");
    auto init = picker.as<IInitializeWithWindow>(); init->Initialize(GetActiveWindow());
    StorageFile file = co_await picker.PickSingleFileAsync();
    if (file) { co_await ShowImageAsync(file); }
}

void MainWindow::Preferences_Click(IInspectable const&, RoutedEventArgs const&) {
    ContentDialog dialog; dialog.Title(box_value(L"Preferences")); dialog.Content(box_value(L"Preferences placeholder: object/face model, box style, and shortcut configuration.")); dialog.CloseButtonText(L"Close"); dialog.XamlRoot(Content().as<FrameworkElement>().XamlRoot()); dialog.ShowAsync();
}
void MainWindow::Exit_Click(IInspectable const&, RoutedEventArgs const&) { Close(); }
void MainWindow::AnalyzeImage_Click(IInspectable const&, RoutedEventArgs const&) { AddDetectionBox(80, 60, 180, 180); AddDetectionBox(320, 120, 220, 150); }
void MainWindow::DetectPeople_Click(IInspectable const&, RoutedEventArgs const&) { OverlayCanvas().Children().Clear(); AddDetectionBox(90, 80, 120, 250); AddDetectionBox(290, 75, 110, 260); }
void MainWindow::DetectVehicles_Click(IInspectable const&, RoutedEventArgs const&) { OverlayCanvas().Children().Clear(); AddDetectionBox(110, 210, 260, 130); }
void MainWindow::DetectAnimals_Click(IInspectable const&, RoutedEventArgs const&) { OverlayCanvas().Children().Clear(); AddDetectionBox(210, 160, 170, 120); }
void MainWindow::DetectTextRegions_Click(IInspectable const&, RoutedEventArgs const&) { OverlayCanvas().Children().Clear(); AddDetectionBox(60, 40, 320, 70); AddDetectionBox(70, 330, 300, 62); }
void MainWindow::ThumbSizeSmall_Click(IInspectable const&, RoutedEventArgs const&) { SetThumbnailSize(96); }
void MainWindow::ThumbSizeMedium_Click(IInspectable const&, RoutedEventArgs const&) { SetThumbnailSize(144); }
void MainWindow::ThumbSizeLarge_Click(IInspectable const&, RoutedEventArgs const&) { SetThumbnailSize(220); }

IAsyncAction MainWindow::LoadFolderAsync(StorageFolder const& folder) {
    m_items.Clear(); m_files.clear();
    auto files = co_await folder.GetFilesAsync();
    for (auto const& file : files) {
        auto ext = to_lower(file.FileType().c_str());
        if (ext == L".png" || ext == L".jpg" || ext == L".jpeg" || ext == L".gif" || ext == L".svg") {
            m_files.push_back(file);
            auto panel = StackPanel(); panel.Orientation(Orientation::Vertical);
            auto img = Image(); img.Width(ThumbSizeSlider().Value()); img.Height(ThumbSizeSlider().Value()); img.Stretch(Stretch::UniformToFill); img.Source(co_await CreateThumbnailAsync(file));
            auto txt = TextBlock(); txt.Text(file.Name()); txt.TextWrapping(TextWrapping::WrapWholeWords); txt.MaxWidth(ThumbSizeSlider().Value());
            panel.Children().Append(img); panel.Children().Append(txt); m_items.Append(panel);
        }
    }
}

IAsyncOperation<ImageSource> MainWindow::CreateThumbnailAsync(StorageFile const& file) {
    auto ext = to_lower(file.FileType().c_str());
    if (ext == L".svg") { auto source = SvgImageSource(); auto stream = co_await file.OpenAsync(FileAccessMode::Read); co_await source.SetSourceAsync(stream); co_return source; }
    StorageItemThumbnail thumb = co_await file.GetThumbnailAsync(ThumbnailMode::PicturesView, static_cast<uint32_t>(ThumbSizeSlider().Value()), ThumbnailOptions::UseCurrentScale);
    auto bitmap = BitmapImage(); co_await bitmap.SetSourceAsync(thumb); co_return bitmap;
}

fire_and_forget MainWindow::ShowImageAsync(StorageFile const& file) {
    auto stream = co_await file.OpenAsync(FileAccessMode::Read);
    auto bmp = BitmapImage(); co_await bmp.SetSourceAsync(stream);
    PreviewImage().Source(bmp); OverlayCanvas().Children().Clear();
    co_await PopulateMetadataAsync(file);
}

void MainWindow::ThumbSizeSlider_ValueChanged(IInspectable const&, Controls::Primitives::RangeBaseValueChangedEventArgs const&) { RefreshThumbnailSizes(); }
void MainWindow::ImageGrid_ItemClick(IInspectable const&, ItemClickEventArgs const& args) {
    uint32_t i{}; if (m_items.IndexOf(args.ClickedItem(), i) && i < m_files.size()) { ShowImageAsync(m_files[i]); }
}

void MainWindow::ImageScrollViewer_PointerWheelChanged(IInspectable const&, PointerRoutedEventArgs const& e) {
    auto delta = e.GetCurrentPoint(ImageScrollViewer()).Properties().MouseWheelDelta();
    m_zoom = std::clamp(m_zoom + (delta > 0 ? 0.1 : -0.1), 0.2, 5.0);
    PreviewImage().RenderTransform(ScaleTransform()); PreviewImage().RenderTransform().as<ScaleTransform>().ScaleX(m_zoom); PreviewImage().RenderTransform().as<ScaleTransform>().ScaleY(m_zoom);
}
void MainWindow::ImageScrollViewer_PointerPressed(IInspectable const&, PointerRoutedEventArgs const& e) {
    if (e.GetCurrentPoint(ImageScrollViewer()).Properties().IsMiddleButtonPressed()) { m_isPanning = true; m_lastPoint = e.GetCurrentPoint(ImageScrollViewer()).Position(); }
}
void MainWindow::ImageScrollViewer_PointerMoved(IInspectable const&, PointerRoutedEventArgs const& e) {
    if (!m_isPanning) return; auto p = e.GetCurrentPoint(ImageScrollViewer()).Position();
    ImageScrollViewer().ChangeView(ImageScrollViewer().HorizontalOffset() - (p.X - m_lastPoint.X), ImageScrollViewer().VerticalOffset() - (p.Y - m_lastPoint.Y), nullptr);
    m_lastPoint = p;
}
void MainWindow::ImageScrollViewer_PointerReleased(IInspectable const&, PointerRoutedEventArgs const&) { m_isPanning = false; }

void MainWindow::OverlayCanvas_PointerPressed(IInspectable const&, PointerRoutedEventArgs const& e) {
    if (!e.GetCurrentPoint(OverlayCanvas()).Properties().IsRightButtonPressed()) return;
    m_isDrawing = true; m_startPoint = e.GetCurrentPoint(OverlayCanvas()).Position();
    m_currentRect = Rectangle(); m_currentRect.Stroke(SolidColorBrush(Windows::UI::Colors::Orange())); m_currentRect.StrokeThickness(2); m_currentRect.StrokeDashArray(DoubleCollection());
    m_currentRect.StrokeDashArray().Append(6); m_currentRect.StrokeDashArray().Append(4);
    OverlayCanvas().Children().Append(m_currentRect); Canvas::SetLeft(m_currentRect, m_startPoint.X); Canvas::SetTop(m_currentRect, m_startPoint.Y);
}
void MainWindow::OverlayCanvas_PointerMoved(IInspectable const&, PointerRoutedEventArgs const& e) {
    if (!m_isDrawing || !m_currentRect) return; auto p = e.GetCurrentPoint(OverlayCanvas()).Position();
    auto x = std::min(m_startPoint.X, p.X), y = std::min(m_startPoint.Y, p.Y); auto w = std::abs(p.X - m_startPoint.X), h = std::abs(p.Y - m_startPoint.Y);
    Canvas::SetLeft(m_currentRect, x); Canvas::SetTop(m_currentRect, y); m_currentRect.Width(w); m_currentRect.Height(h);
}
void MainWindow::OverlayCanvas_PointerReleased(IInspectable const&, PointerRoutedEventArgs const&) { m_isDrawing = false; }

void MainWindow::AddDetectionBox(float x, float y, float w, float h) {
    auto rect = Rectangle(); rect.Width(w); rect.Height(h); rect.Stroke(SolidColorBrush(Windows::UI::Colors::LimeGreen())); rect.StrokeThickness(2);
    rect.StrokeDashArray(DoubleCollection()); rect.StrokeDashArray().Append(4); rect.StrokeDashArray().Append(3);
    OverlayCanvas().Children().Append(rect); Canvas::SetLeft(rect, x); Canvas::SetTop(rect, y);
}

void MainWindow::RefreshThumbnailSizes() {
    for (uint32_t i = 0; i < m_items.Size(); ++i) {
        auto panel = m_items.GetAt(i).as<StackPanel>();
        auto image = panel.Children().GetAt(0).as<Image>();
        auto text = panel.Children().GetAt(1).as<TextBlock>();
        image.Width(ThumbSizeSlider().Value());
        image.Height(ThumbSizeSlider().Value());
        text.MaxWidth(ThumbSizeSlider().Value());
    }
}

void MainWindow::SetThumbnailSize(double size) {
    ThumbSizeSlider().Value(size);
    RefreshThumbnailSizes();
}

IAsyncAction MainWindow::PopulateMetadataAsync(StorageFile const& file) {
    FileNameValue().Text(file.Name());
    FilePathValue().Text(file.Path());
    auto basicProps = co_await file.GetBasicPropertiesAsync();
    FileSizeValue().Text(to_hstring(basicProps.Size()) + L" bytes");
    Windows::Globalization::DateTimeFormatting::DateTimeFormatter dtf(L"{month.full} {day.integer}, {year.full} {hour.integer}:{minute.integer(2)}");
    FileModifiedValue().Text(dtf.Format(basicProps.DateModified()));
    FileTypeValue().Text(file.FileType());
}
void MainWindow::ShowBoxMenu(float, float) {
    MenuFlyout flyout; flyout.Items().Append(MenuFlyoutItem()); flyout.Items().GetAt(0).as<MenuFlyoutItem>().Text(L"Crop to Box");
    flyout.Items().Append(MenuFlyoutItem()); flyout.Items().GetAt(1).as<MenuFlyoutItem>().Text(L"Copy to Clipboard");
    flyout.Items().Append(MenuFlyoutItem()); flyout.Items().GetAt(2).as<MenuFlyoutItem>().Text(L"Open in New Image");
    flyout.ShowAt(OverlayCanvas());
}
void MainWindow::OverlayCanvas_RightTapped(IInspectable const&, RightTappedRoutedEventArgs const& e) { ShowBoxMenu(e.GetPosition(OverlayCanvas()).X, e.GetPosition(OverlayCanvas()).Y); }
}
