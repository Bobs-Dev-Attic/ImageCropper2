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
#include <tuple>
#include <sstream>

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
void MainWindow::AnalyzeImage_Click(IInspectable const&, RoutedEventArgs const&) {
    ClearAndRenderDetections({
        {72, 48, 128, 128, L"Face", 0.96},
        {84, 178, 110, 170, L"Upper Body", 0.91},
        {292, 118, 220, 156, L"Animal", 0.87},
        {320, 280, 70, 44, L"Animal Head", 0.83},
        {128, 340, 44, 44, L"Hand", 0.76}
    });
}
void MainWindow::DetectPeople_Click(IInspectable const&, RoutedEventArgs const&) {
    ClearAndRenderDetections({
        {90, 80, 120, 250, L"Person", 0.92},
        {290, 75, 110, 260, L"Person", 0.89}
    });
}
void MainWindow::DetectVehicles_Click(IInspectable const&, RoutedEventArgs const&) { ClearAndRenderDetections({ {110, 210, 260, 130, L"Vehicle", 0.88} }); }
void MainWindow::DetectAnimals_Click(IInspectable const&, RoutedEventArgs const&) {
    ClearAndRenderDetections({
        {210, 160, 170, 120, L"Animal", 0.9},
        {258, 130, 66, 54, L"Animal Head", 0.84}
    });
}
void MainWindow::DetectBodyParts_Click(IInspectable const&, RoutedEventArgs const&) {
    ClearAndRenderDetections({
        {92, 78, 112, 118, L"Head", 0.93},
        {86, 198, 124, 118, L"Torso", 0.9},
        {70, 212, 28, 92, L"Left Arm", 0.81},
        {202, 214, 28, 92, L"Right Arm", 0.8},
        {112, 322, 38, 82, L"Left Leg", 0.84},
        {156, 324, 38, 82, L"Right Leg", 0.83}
    });
}
void MainWindow::DetectTextRegions_Click(IInspectable const&, RoutedEventArgs const&) {
    ClearAndRenderDetections({
        {60, 40, 320, 70, L"Text", 0.86},
        {70, 330, 300, 62, L"Text", 0.82}
    });
}
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

void MainWindow::AddDetectionBox(float x, float y, float w, float h, hstring const& label, double confidence) {
    auto rect = Rectangle(); rect.Width(w); rect.Height(h); rect.Stroke(SolidColorBrush(Windows::UI::Colors::LimeGreen())); rect.StrokeThickness(2);
    rect.StrokeDashArray(DoubleCollection()); rect.StrokeDashArray().Append(4); rect.StrokeDashArray().Append(3);
    OverlayCanvas().Children().Append(rect); Canvas::SetLeft(rect, x); Canvas::SetTop(rect, y);

    auto caption = TextBlock();
    std::wstringstream ss;
    ss << label.c_str();
    if (confidence > 0.0) {
        ss << L" (" << static_cast<int>(confidence * 100.0 + 0.5) << L"%)";
    }
    caption.Text(ss.str());
    caption.Foreground(SolidColorBrush(Windows::UI::Colors::White()));
    caption.Background(SolidColorBrush(Windows::UI::Colors::ForestGreen()));
    caption.Padding(ThicknessHelper::FromLengths(6, 2, 6, 2));
    caption.FontSize(12);
    OverlayCanvas().Children().Append(caption);
    Canvas::SetLeft(caption, x);
    Canvas::SetTop(caption, std::max(0.0f, y - 22.0f));
}

void MainWindow::ClearAndRenderDetections(std::vector<std::tuple<float, float, float, float, hstring, double>> const& detections) {
    OverlayCanvas().Children().Clear();
    for (auto const& detection : detections) {
        AddDetectionBox(std::get<0>(detection), std::get<1>(detection), std::get<2>(detection), std::get<3>(detection), std::get<4>(detection), std::get<5>(detection));
    }
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
