using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows;
using System.Timers;
using System.Windows.Media;
using Cosmos.App.Sdk.v1;

namespace Cosmos.App.Hithink.ComDemo
{
    class WpfToast : ToolTip
    {
        class ToastBrushes
        {
            public ToastBrushes(String backgroundString, String foregroundString)
            {
                Background = _createBrush(backgroundString);
                Foreground = _createBrush(foregroundString);
            }
            public Brush Background { get; }
            public Brush Foreground { get; }
        }
        private static SolidColorBrush _createBrush(string colorString)
        {
            var color = (Color)ColorConverter.ConvertFromString(colorString);
            var brush = new SolidColorBrush(color);
            brush.Freeze();
            return brush;
        }
        private static IDictionary<CosmosLogLevel, ToastBrushes> toastBrushes { get; }
        static WpfToast()
        {
            toastBrushes = new Dictionary<CosmosLogLevel, ToastBrushes>()
            {
                { CosmosLogLevel.Information, new ToastBrushes("#c6efce", "#006100") },
                { CosmosLogLevel.Error, new ToastBrushes("#ffc7ce", "#9c0006") },
                { CosmosLogLevel.Warning, new ToastBrushes("#ffeb9c", "#9c5700") },
            };
        }
        private WpfToast()
        {
            SetResourceReference(StyleProperty, typeof(ToolTip));
        }
        public static void Show(string content, CosmosLogLevel logLevel = CosmosLogLevel.Information, int milliseconds = 2000, UIElement parentElement = null)
        {
            var levelBrushes = toastBrushes[logLevel];

            var contentTextBlock = new TextBlock()
            {
                HorizontalAlignment = HorizontalAlignment.Center,
                VerticalAlignment = VerticalAlignment.Center,
                Text = content
            };
            var toast = new WpfToast()
            {
                Background = levelBrushes.Background,
                Foreground = levelBrushes.Foreground,
                Content = contentTextBlock,
            };
            toast.fadeOutTimer = new System.Timers.Timer();
            toast.fadeOutTimer.Interval = milliseconds;
            toast.fadeOutTimer.Elapsed += (sender, e) =>
            {
                toast.fadeOutTimer.Stop();
                toast.fadeOutTimer.Dispose();
                toast.Dispatcher.Invoke(() =>
                {
                    toast.IsOpen = false;
                });
            };
            toast.PlacementTarget = parentElement;
            toast.fadeOutTimer.Start();
            toast.IsOpen = true;
        }


        private System.Timers.Timer fadeOutTimer { get; set; }

    }
}
