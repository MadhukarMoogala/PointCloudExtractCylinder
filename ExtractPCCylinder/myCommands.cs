// (C) Copyright 2017 by  
//
using System;
using Autodesk.AutoCAD.Runtime;
using Autodesk.AutoCAD.ApplicationServices;
using Autodesk.AutoCAD.DatabaseServices;
using Autodesk.AutoCAD.Geometry;
using Autodesk.AutoCAD.EditorInput;
using System.Runtime.InteropServices;

// This line is not mandatory, but improves loading performances
[assembly: CommandClass(typeof(ExtractPCCylinder.MyCommands))]

namespace ExtractPCCylinder
{

    // This class is instantiated by AutoCAD for each document when
    // a command is called by the user the first time in the context
    // of a given document. In other words, non static data in this class
    // is implicitly per-document!
    public class MyCommands
    {
        [DllImport("D:\\ARXProjects\\PCExtractCylinder\\x64\\Debug\\PCExtractCylinder.arx", CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Unicode,
            EntryPoint = "getCylinder")]
        private static extern ErrorStatus getCylinder(IntPtr oldId, ref Elements elements);


        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public struct Elements
        {
            public Vector3d _axis;
            public Point3d _origin;
            public double _height;
            public double _raidus;

        }
        // Modal Command with localized name
        [CommandMethod("TGroup", "TExtactCyclinder", "TExtactCyclinder", CommandFlags.Modal)]
        public void tExtract() // This method can have any name
        {
            // Put your command code here
            Document doc = Application.DocumentManager.MdiActiveDocument;
            Editor ed;
            if (doc != null)
            {
                ed = doc.Editor;
               PromptSelectionResult psr =  ed.GetSelection();
                if (psr.Status != PromptStatus.OK) return;

                using (Transaction t = doc.Database.TransactionManager.StartOpenCloseTransaction())
                {
                    Database db = doc.Database;
                    SelectionSet acSSet = psr.Value;

                    // Step through the objects in the selection set
                    foreach (SelectedObject acSSObj in acSSet)
                    {
                        // Check to make sure a valid SelectedObject object was returned
                        if (acSSObj != null)
                        {
                            // Open the selected object for write
                            PointCloudEx acEnt = t.GetObject(acSSObj.ObjectId,OpenMode.ForRead) as PointCloudEx;

                            if (acEnt != null)
                            {

                                Elements elements = new Elements();
                                ErrorStatus es = getCylinder(acEnt.ObjectId.OldIdPtr, ref elements);
                                if (es == ErrorStatus.OK)
                                {
                                    ed.WriteMessage("_axis {0},origin {1},height {2},raidus {3}" 
                                                    + elements._axis.ToString()+"\t" 
                                                    + elements._origin.ToString()+"\t" 
                                                    + elements._height +"\t"
                                                    + elements._raidus);
                                }
                                else
                                {
                                    ed.WriteMessage(es.ToString());
                                }


                                break;
                            }
                        }
                    }
                }

                    ed.WriteMessage("Hello, this is your first command.");

            }
        }

      

    }

}
