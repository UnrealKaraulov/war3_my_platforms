using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.IO;

namespace Client.War3
{
     class TriggerStringParser
    {

        const string GetTrigStrID = @"^\s*STRING\s+(\w+)";
        const string GetTrigStrStart = @"^\s*{";
        const string GetTrigStrEnd = @"^\s*}";


        public TriggerStringParser(string Path)
        {
            ParseFile(Path);
        }

        public TriggerStringParser()
        {

        }

        struct TriggerStr
        {
            public string id;
            public string text;
        }

        List<TriggerStr> TriggerStrList = new List<TriggerStr>();

        public string GetTrigStrText(string trigid)
        {
            if (trigid.ToLower().IndexOf("trigstr_") > -1)
            {
                trigid = trigid.ToLower().Replace("trigstr_", "");

                foreach (var s in TriggerStrList)
                {
                    if (s.id == trigid)
                        return s.text;
                }

            }
            return trigid;
        }


        public void ParseFile(string path)
        {
            TriggerStrList.Clear();

            try
            {
                string[] list = File.ReadAllLines(path);
                TriggerStr triggerStr = new TriggerStr();
                bool StrStarted = false;
                int StrId = 0;
                foreach (string s in list)
                {
                    Match trigstr_started = null;
                    if ((trigstr_started = Regex.Match(s, GetTrigStrID)).Success)
                    {
                        triggerStr.id = trigstr_started.Groups[1].Value;
                    }
                    if (Regex.Match(s, GetTrigStrStart).Success)
                    {
                        StrStarted = true;
                        triggerStr.text = "";
                        StrId = 0;
                    }
                    else if (Regex.Match(s, GetTrigStrEnd).Success)
                    {
                        StrStarted = false;
                        TriggerStrList.Add(triggerStr);
                    }
                    else if (StrStarted )
                    {
                        if (StrId > 0)
                            triggerStr.text += "\n";
                        triggerStr.text += s;
                        StrId++;
                    }

             

                }
            }
            catch
            {

            }
        }

    }
}
