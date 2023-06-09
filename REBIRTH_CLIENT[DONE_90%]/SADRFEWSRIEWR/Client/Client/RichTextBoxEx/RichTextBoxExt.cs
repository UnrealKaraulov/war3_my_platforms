﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;

namespace DemoApp
{
    
    public class RichTextBoxExt : RichTextBox
    {
        private DispatcherTimer _timer;

        public RichTextBoxExt()
        {
            Emoticons = new EmoticonCollection();
        }

        /// <summary>
        /// Override to trigger the look up.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnTextChanged(TextChangedEventArgs e)
        {
            //Looking for an idle time to start look up..
            if (_timer == null)
            {
                _timer = new DispatcherTimer(DispatcherPriority.Background);
                _timer.Interval = TimeSpan.FromSeconds(0.3);
                _timer.Tick += LookUp;
            }

            //Restart timer here...
            _timer.Stop();
            _timer.Start();

            base.OnTextChanged(e);
        }

        private void LookUp(object sender, EventArgs e)
        {
             Dispatcher.BeginInvoke((Action)(() => { UpdateSmileys( ); }));
            _timer.Stop();
        }

        /// <summary>
        /// Iterate through words and get the text range of smiley text. Replace it with corresponding icon.
        /// </summary>
        private void UpdateSmileys()
        {
            var tp = Document.ContentStart;
            var word = WordBreaker.GetWordRange(tp);
            
            while (word.End.GetNextInsertionPosition(LogicalDirection.Forward) != null)
            {
                word = WordBreaker.GetWordRange(word.End.GetNextInsertionPosition(LogicalDirection.Forward));
                var smileys = from smiley in Emoticons
                              where smiley.Text == word.Text
                              select smiley;

                var emoticonMappers = smileys as IList<EmoticonMapper> ?? smileys.ToList();

                if (emoticonMappers.Any())
                {
                    var emoticon = emoticonMappers.FirstOrDefault();
                    var img = new Image(){Stretch = Stretch.None};
                    if (emoticon != null) img.Source = emoticon.Icon;
                         ReplaceTextRangeWithImage(word, img);
                }
            }
        
        }

        /// <summary>
        /// Replacing the text range with image.
        /// </summary>
        /// <param name="textRange">The smiley text range.</param>
        /// <param name="image">The smiley icon</param>
        public void ReplaceTextRangeWithImage(TextRange textRange, Image image)
        {
            if (textRange.Start.Parent is Run)
            {
                var run = textRange.Start.Parent as Run;

                var runBefore =
                    new Run(new TextRange(run.ContentStart, textRange.Start).Text);
                var runAfter =
                    new Run(new TextRange(textRange.End, run.ContentEnd).Text);

                image.DataContext = textRange.Text;

                if (textRange.Start.Paragraph != null)
                {
                    textRange.Start.Paragraph.Inlines.Add(runBefore);
                    textRange.Start.Paragraph.Inlines.Add(image);
                  //  textRange.Start.Paragraph.Inlines.Add(textRange.Text);
                    textRange.Start.Paragraph.Inlines.Add(runAfter);
                    textRange.Start.Paragraph.Inlines.Remove(run);
                }

                CaretPosition = runAfter.ContentEnd;

            }
        }

        /// <summary>
        /// The collection of Emoticon mappers
        /// </summary>
        public EmoticonCollection Emoticons { get; set; }

        public void AppendText(string text, string color)
        {
            BrushConverter bc = new BrushConverter();
            TextRange tr = new TextRange(this.Document.ContentEnd, this.Document.ContentEnd);
            tr.Text = text;
            var OldColor = tr.GetPropertyValue(TextElement.ForegroundProperty);

            try
            {
                tr.ApplyPropertyValue(TextElement.ForegroundProperty,
                    bc.ConvertFromString(color));
            }
            catch (FormatException)
            {
                tr.ApplyPropertyValue(TextElement.ForegroundProperty, Brushes.White);
            }

            tr = new TextRange(this.Document.ContentEnd, this.Document.ContentEnd);


            tr.Text = "\a";
            try
            {
                tr.ApplyPropertyValue(TextElement.ForegroundProperty, OldColor);
            }
            catch
            {

            }

        }

        public void AppendText(string text, string color, bool bold)
        {
            BrushConverter bc = new BrushConverter();
            TextRange tr = new TextRange(this.Document.ContentEnd, this.Document.ContentEnd);
            tr.Text = text;


            var OldWeight = tr.GetPropertyValue(TextElement.FontWeightProperty);
            var OldColor = tr.GetPropertyValue(TextElement.ForegroundProperty);

            tr.ApplyPropertyValue(TextElement.FontWeightProperty, bold ? FontWeights.Bold : FontWeights.Regular);



            try
            {
                tr.ApplyPropertyValue(TextElement.ForegroundProperty,
                    bc.ConvertFromString(color));
            }
            catch (FormatException)
            {
                tr.ApplyPropertyValue(TextElement.ForegroundProperty, Brushes.White);
            }

            tr = new TextRange(this.Document.ContentEnd, this.Document.ContentEnd);
            tr.Text = "\a";
            try
            {
                tr.ApplyPropertyValue(TextElement.FontWeightProperty, OldWeight);
            }
            catch
            {

            }
            try
            {
                tr.ApplyPropertyValue(TextElement.ForegroundProperty, OldColor);
            }
            catch
            {

            }


        }
    }

}

