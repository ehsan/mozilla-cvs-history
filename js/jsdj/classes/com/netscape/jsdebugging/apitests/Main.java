/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

package com.netscape.jsdebugging.apitests;

import java.io.*;
import java.util.*;
import com.netscape.jsdebugging.engine.*;
import com.netscape.jsdebugging.api.*;
import com.netscape.jsdebugging.apitests.xml.*;
import com.netscape.jsdebugging.apitests.testing.*;

/**
 * This class runs a set of tests on the given engine.
 *
 * @author Alex Rakhlin
 */

public class Main {
    
    public static final String LOCAL_CREATOR_NAME = 
        "com.netscape.jsdebugging.engine.local.RuntimeCreatorLocal";
    public static final String RHINO_CREATOR_NAME = 
        "com.netscape.jsdebugging.engine.rhino.RuntimeCreatorRhino";
        
        
    public static void main(String args[]) {        
        new Main(args);
    }
    
    private Main (String args[]) {            
        
        Tags.init();
        _processArguments (args);
        
        _engine_name = "";
        if (_creatorName.equals (LOCAL_CREATOR_NAME)) _engine_name = "SPIDER_MONKEY";
        if (_creatorName.equals (RHINO_CREATOR_NAME)) _engine_name = "RHINO";
        try {
            System.setErr (new PrintStream (new FileOutputStream ("error_main_"+_engine_name+".log")));
        } catch (IOException e) { System.out.println ("Error redirecting error stream"); }
        
        try {
            System.setOut (new PrintStream (new FileOutputStream ("output_main_"+_engine_name+".log")));
        } catch (IOException e) { System.out.println ("Error redirecting error stream"); }
        

        if ( null == _xmlw ){
                System.out.println ("FAILED to Init Output");
                return;
        }
        
        try {
            Class clazz = Class.forName(_creatorName);
            IRuntimeCreator creator = (IRuntimeCreator) clazz.newInstance();
            _runtime = creator.newRuntime(IRuntimeCreator.ENABLE_DEBUGGING, null);
            _context = _runtime.newContext(null);
            _controller = _runtime.getDebugController();
            _sourceTextProvider = _runtime.getSourceTextProvider();
            LocalSink sink = new LocalSink();
            _context.setPrintSink(sink);
            _context.setErrorSink(sink);

            if( null == _controller )
            {
                System.out.println ("FAILED to Init DebugController");
                return;
            }
            
            _xmlw.startTag ("TESTS");
            _tests = new TestSuite (_xmlw, _controller);
            for (int i = 0; i < _test_names.size(); i++){
                Test t = null;
                try {
                    t = (Test) java.lang.Class.forName ((String)_test_names.elementAt(i)).newInstance();
                }
                catch (ClassNotFoundException e) { System.err.println ("CLASS "+(String)_test_names.elementAt(i)+" not found");}
                _xmlw.tag ("TEST", (String)_test_names.elementAt(i));
                t.setXMLWriter (_xmlw);
                _tests.addTest (t);
            }
            _xmlw.endTag ("TESTS");
            _xmlw.startTag ("SCRIPT_FILES");
            for (int i = 0; i < _filenames.size(); i++){
                _xmlw.tag ("FILE", (String) _filenames.elementAt (i));
            }   
            _xmlw.endTag ("SCRIPT_FILES");
            _controller.setScriptHook   (new MyScriptHook   (_tests));
            _controller.setInterruptHook(new MyInterruptHook(_tests));
            _controller.setErrorReporter(new MyErrorReporter(_tests));
            _controller.sendInterrupt();
            
        }
        catch(Throwable t)
        {
            System.err.println("execption thrown " + t );
        }
        
        _xmlw.tag ("ENGINE", _engine_name);
        
        //-------------------- LOOP -----------------
        for (int i = 0; i < _filenames.size(); i++){
            _context.loadFile((String) _filenames.elementAt (i));
        }
        //--------------------- END -----------------

        _xmlw.close();
        /*
        try {
            DataInputStream d = new DataInputStream (System.in);
            d.readLine();
        }
        catch (IOException e) {}
        */
    }


    public String getClassName() {
        return "global";
    }


    private void _processArguments (String args[]){
        _filenames = new Vector();
        _test_names = new Vector();
        _creatorName = null;
        
        /* Process command line arguments
        */
        for (int i=0; i < args.length; i++) {
            String arg = args[i];
            
            if (arg.equals("-f")) {
                for (int j=i+1; j < args.length; j++)
                   if (!args[j].startsWith ("-")) _filenames.addElement(args[j]);
                   else break;
            }
            if (arg.equals("-o")){
                _xmlw = new XMLWriter (args[i+1], 2); //second arg is the starting indent
            }
            if (arg.equals("-t")){
                for (int j=i+1; j < args.length; j++)
                   if (!args[j].startsWith ("-")) _test_names.addElement(args[j]);
                   else break;
            }
            if (arg.equals ("-e")){
                if (args[i+1].equalsIgnoreCase ("Rhino")) _creatorName = RHINO_CREATOR_NAME;
                if (args[i+1].equalsIgnoreCase ("Local")) _creatorName = LOCAL_CREATOR_NAME;
            }
        }
    }
    
    public DebugController getController()    {return _controller;}
    public SourceTextProvider getSourceTextProvider() {return _sourceTextProvider;}

    private DebugController _controller;
    private IRuntime _runtime;
    private IContext _context;
    private SourceTextProvider _sourceTextProvider;

    private Vector _filenames;
    private XMLWriter _xmlw;
    private TestSuite _tests;
    private Vector _test_names;
    private String _creatorName;;
    private String _engine_name;
}

class LocalSink implements IPrintSink, IErrorSink
{
    public void print(String s) {
        System.out.print(s);
    }
    public void error(String msg, String filename, int lineno,
                      String lineBuf, int offset)
    {
        System.out.println("");
        System.out.println("-------------------------------------");
        System.out.println(msg);
        System.out.println(filename);
        System.out.println(lineno);
        System.out.println(lineBuf);
        System.out.println(offset);
        System.out.println("-------------------------------------");
        System.out.println("");
    }
}