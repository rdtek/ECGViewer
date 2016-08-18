using System;
using System.Collections.Generic;
using System.Linq;
using System.Xml;
using System.Collections.ObjectModel;

namespace ECGViewerWPF
{
    /*****************************************************************************************************
    * Class: SignalBeatFamily
    ******************************************************************************************************/
    public class SignalBeatFamily
    {
        public int ID { get; set; }
        public string Signature { get; set; }
        public int NumberOfBeats { get; set; }

        public SignalBeatFamily() { }
    }

    /******************************************************************************************************
    * Class: SignalBeat
    /******************************************************************************************************/
    public class SignalBeat
    {
        public int FamilyID { get; set; }
        public string Label { get; set; }
        public int Time { get; set; }
        public int TimeOffset { get; set; }

        public SignalBeat() { }
        public SignalBeat(XmlAttributeCollection attr)
        {
            FamilyID = Int32.Parse(attr["FamilyID"].Value);
            Label = attr["Label"].Value;
            Time = Int32.Parse(attr["Time"].Value);
            TimeOffset = Int32.Parse(attr["TimeOffset"].Value);
        }
        public SignalBeat(int familyID, string label, int time, int timeOffset)
        {
            FamilyID = familyID;
            Label = label;
            Time = time;
            TimeOffset = timeOffset;
        }
        public override string ToString()
        {
            return "" + FamilyID + " " + Label + " " + Time + " " + TimeOffset;
        }
    }

    /******************************************************************************************************
    * Class: SignalAnalysisResults
    /******************************************************************************************************/
    public class SignalAnalysis
    {
        SignalBeat[] m_beats;
        SignalBeatFamily[] m_beatFamilies;
        string[] m_signalLeads;
        string m_summaryInfo = "";

        public SignalAnalysis() { }

        public SignalAnalysis(string xmlFilePath)
        {
            LoadDataFromXml(xmlFilePath);
        }

        public bool HasResults
        {
            get { return m_beats != null && m_beats.Length >= 1; }
        }

        private void LoadDataFromXml(string xmlFilePath)
        {
            var beats = new List<SignalBeat>();
            var beatFamilies = new List<SignalBeatFamily>();

            XmlDocument doc = new XmlDocument();

            doc.Load(xmlFilePath);

            XmlNode leadNode = doc.SelectSingleNode("/SignalAnalysisResults/AnalyzedLeads");
            if (leadNode != null) m_signalLeads = leadNode.InnerText.Split(new char[] { ',' });

            XmlNodeList beatNodes = doc.SelectNodes("/SignalAnalysisResults/Beats/Beat");
            foreach (XmlNode node in beatNodes)
            {
                SignalBeat beat = new SignalBeat(node.Attributes);
                beats.Add(beat);
            }

            XmlNodeList beatFamilyNodes = doc.SelectNodes("/SignalAnalysisResults/BeatFamilies/BeatFamily");
            foreach (XmlNode node in beatFamilyNodes)
            {
                SignalBeatFamily beatFam = new SignalBeatFamily();
                beatFam.ID = Int32.Parse(node.Attributes["ID"].Value);
                beatFam.Signature = node.Attributes["Signature"].Value;
                beatFam.NumberOfBeats = Int32.Parse(node.Attributes["NumberOfBeats"].Value);
                beatFamilies.Add(beatFam);
            }

            m_summaryInfo = (m_signalLeads != null) ? "Leads: " + string.Join(",", m_signalLeads) : "";

            m_summaryInfo += " Total beats:  " + beatNodes.Count
                + " Total families: " + beatFamilyNodes.Count;

            m_beatFamilies = beatFamilies.ToArray();
            m_beats = beats.ToArray();
        }

        public SignalBeatFamily[] GetBeatFamilies()
        {
            return m_beatFamilies != null
                ? m_beatFamilies : new List<SignalBeatFamily>().ToArray();
        }

        public SignalBeat[] GetBeats(int limit = 0)
        {
            return m_beats != null
                ? m_beats : new List<SignalBeat>().ToArray();
        }

        public string ToXml()
        {
            string strXml = "<SignalAnalysisResults>\r\n";
            SignalBeat[] beats = GetBeats();
            SignalBeatFamily[] families = GetBeatFamilies();
            string tab = "    ";//4 spaces tab

            strXml += tab + "<AnalyzedLeads>" + string.Join(",", m_signalLeads) + "</AnalyzedLeads>\r\n";

            //Beat families
            strXml += tab + "<BeatFamilies>\r\n";
            foreach (SignalBeatFamily f in families)
            {
                strXml += string.Format(
                    tab + tab + "<BeatFamily ID=\"{0}\" Signature=\"{1}\" NumberOfBeats=\"{2}\" />\r\n",
                    f.ID, f.Signature, f.NumberOfBeats);
            }
            strXml += tab + "</BeatFamilies>\r\n";

            //Beats
            strXml += tab + "<Beats>\r\n";
            foreach (SignalBeat beat in beats)
            {
                strXml += string.Format(
                    tab + tab + "<Beat FamilyID=\"{0}\" Label=\"{1}\" Time=\"{2}\" TimeOffset=\"{3}\" />\r\n",
                    beat.FamilyID, beat.Label, beat.Time, beat.TimeOffset);
            }
            strXml += tab + "</Beats>\r\n";

            strXml += "</SignalAnalysisResults>";

            return strXml;
        }

        public ObservableCollection<object> ToDataRows()
        {
            ObservableCollection<object> rows = new ObservableCollection<object>();
            foreach (SignalBeat beat in GetBeats())
            {
                rows.Add(new
                {
                    FamilyID = beat.FamilyID,
                    BeatType = beat.Label,
                    Start = beat.Time,
                    Offset = beat.TimeOffset
                });
            }
            return rows;
        }

        public override string ToString()
        {
            return m_summaryInfo;
        }
    }
}
