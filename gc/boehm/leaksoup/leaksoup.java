/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express oqr
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 *
 * Patrick C. Beard <beard@netscape.com>
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Public License (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only
 * under the terms of the GPL and not to allow others to use your
 * version of this file under the NPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL.  If you do not delete
 * the provisions above, a recipient may use your version of this
 * file under either the NPL or the GPL.
 */

import java.io.*;
import java.util.*;

class Leak {
	String mAddress;
	Object[] mReferences;
	int mRefCount;
	int mSize;
	String mType;

	Leak(String addr, String type, Object[] refs, int size) {
		mAddress = addr;
		mReferences = refs;
		mRefCount = 0;
		mSize = size;
		mType = type;
	}

	public String toString() {
		return (mAddress + " [" + mRefCount + "] (" + mSize + ") " + mType);
	}
	
	static class Comparator implements QuickSort.Comparator {
		public int compare(Object obj1, Object obj2) {
			Leak l1 = (Leak) obj1, l2 = (Leak) obj2;
			return (l1.mRefCount - l2.mRefCount);
		}
	}
	
	public static void sort(Leak[] leaks) {
		QuickSort sorter = new QuickSort(new Comparator());
		sorter.sort(leaks);
	}
}

public class leaksoup {
	public static void main(String[] args) {
		if (args.length == 0) {
			System.out.println("usage:  leaksoup leaks");
			System.exit(1);
		}
		
		String inputName = args[0];
	
		try {
			Vector vec = new Vector();
			Hashtable table = new Hashtable();
			Histogram hist = new Histogram();
			BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(inputName)));
			String line = reader.readLine();
			while (line != null) {
				if (line.startsWith("0x")) {
					String addr = line.substring(0, 10);
					String type = line.substring(line.indexOf('<'), line.indexOf('>') + 1);
					int size;
					try {
						String str = line.substring(line.indexOf('(') + 1, line.indexOf(')')).trim();
						size = Integer.parseInt(str);
					} catch (NumberFormatException nfe) {
						size = 0;
					}
					vec.setSize(0);
					for (line = reader.readLine(); line != null && line.charAt(0) == '\t'; line = reader.readLine())
						vec.addElement(line.substring(1, 11));
					Object[] refs = new Object[vec.size()];
					vec.copyInto(refs);
					table.put(addr, new Leak(addr, type, refs, size));
					if (type.equals("<void*>"))
						type = "(" + size + ") <void*>";
					hist.record(type);
				} else {
					line = reader.readLine();
				}
			}
			reader.close();
			
			Leak[] leaks = new Leak[table.size()];
			int leakCount = 0;
			long totalSize = 0;

			// now, we have a table full of leaked objects, lets derive reference counts, and build the graph.
			Enumeration e = table.elements();
			while (e.hasMoreElements()) {
				Leak leak = (Leak) e.nextElement();
				Object[] refs = leak.mReferences;
				int count = refs.length;
				for (int i = 0; i < count; i++) {
					String addr = (String) refs[i];
					Leak ref = (Leak) table.get(addr);
					if (ref != null) {
						// increase the ref count.
						ref.mRefCount++;
						// change string to ref itself.
						refs[i] = ref;
					}
				}
				leaks[leakCount++] = leak;
				totalSize += leak.mSize;
			}
			
			// be nice to the GC.
			table.clear();
			table = null;
			
			// store the leak report in inputName + ".soup"
			PrintStream out = new PrintStream(new FileOutputStream(inputName + ".soup"));
			
			// print the object histogram report.
			printHistogram(out, hist);
			
			// print the leak report.
			printLeaks(out, leaks, leakCount, totalSize);
			
			out.close();
		} catch (Exception e) {
			e.printStackTrace(System.err);
		}
	}
	
	static void printLeaks(PrintStream out, Leak[] leaks, int leakCount, long totalSize) throws IOException {
		// sort the leaks by reference count.
		QuickSort sorter = new QuickSort(new Leak.Comparator());
		sorter.sort(leaks);
		
		// store reduced leaks in "RuntimeLeaks.soup"
		out.println("Leak Soup Report:");
		out.println("total objects leaked = " + leakCount);
		out.println("total memory leaked  = " + totalSize + " bytes.");

		// now, print the report, sorted by reference count.
		for (int i = 0; i < leakCount; i++) {
			Leak leak = leaks[i];
			out.println(leak);
			Object[] refs = leak.mReferences;
			int count = refs.length;
			for (int j = 0; j < count; j++)
				out.println("\t" + refs[j]);
		}
	}
	
	static class HistComparator implements QuickSort.Comparator {
		Histogram hist;
		
		HistComparator(Histogram hist) {
			this.hist = hist;
		}
	
		public int compare(Object obj1, Object obj2) {
			return (hist.count(obj1) - hist.count(obj2));
		}
	}

	static void printHistogram(PrintStream out, Histogram hist) throws IOException {
		// sort the objects by histogram count.
		Object[] objects = hist.objects();
		QuickSort sorter = new QuickSort(new HistComparator(hist));
		sorter.sort(objects);
		
		out.println("Leak Type Histogram:");
		int count = objects.length;
		while (count > 0) {
			Object object = objects[--count];
			out.println(object.toString() + " : " + hist.count(object));
		}
	}
}
