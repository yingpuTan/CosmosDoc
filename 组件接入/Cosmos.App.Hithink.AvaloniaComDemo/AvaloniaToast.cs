using System;
using System.Collections.Generic;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Media;
using Avalonia.Threading;
using Cosmos.App.Sdk.v1;

namespace Cosmos.App.Hithink.AvaloniaComDemo
{
    class AvaloniaToast : Popup
    {
        class ToastBrushes
        {
            public ToastBrushes(String backgroundString, String foregroundString)
            {
                Background = _createBrush(backgroundString);
                Foreground = _createBrush(foregroundString);
            }
            public IBrush Background { get; }
            public IBrush Foreground { get; }
        }
        private static SolidColorBrush _createBrush(string colorString)
        {
            var color = Color.Parse(colorString);
            return new SolidColorBrush(color);
        }
        private static IDictionary<CosmosLogLevel, ToastBrushes> toastBrushes { get; }
        static AvaloniaToast()
        {
            toastBrushes = new Dictionary<CosmosLogLevel, ToastBrushes>()
            {
                { CosmosLogLevel.Information, new ToastBrushes("#c6efce", "#006100") },
                { CosmosLogLevel.Error, new ToastBrushes("#ffc7ce", "#9c0006") },
                { CosmosLogLevel.Warning, new ToastBrushes("#ffeb9c", "#9c5700") },
            };
        }
        private AvaloniaToast()
        {
        }
        public static void Show(string content, CosmosLogLevel logLevel = CosmosLogLevel.Information, int milliseconds = 2000, Control? parentElement = null)
        {
            var levelBrushes = toastBrushes[logLevel];

            var contentTextBlock = new TextBlock()
            {
                HorizontalAlignment = Avalonia.Layout.HorizontalAlignment.Center,
                VerticalAlignment = Avalonia.Layout.VerticalAlignment.Center,
                Text = content
            };
            var toast = new AvaloniaToast()
            {
                Background = levelBrushes.Background,
                Foreground = levelBrushes.Foreground,
                Child = contentTextBlock,
            };
            
            var timer = new DispatcherTimer();
            timer.Interval = TimeSpan.FromMilliseconds(milliseconds);
            timer.Tick += (sender, e) =>
            {
                timer.Stop();
                toast.IsOpen = false;
            };
            
            if (parentElement != null)
            {
                toast.PlacementTarget = parentElement;
            }
            toast.Placement = PlacementMode.Pointer;
            toast.IsOpen = true;
            timer.Start();
        }
    }
}

