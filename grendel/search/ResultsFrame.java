/* -*- Mode: java; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the Grendel mail/news client.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1997
 * Netscape Communications Corporation.  All Rights Reserved.
 *
 * Created: Will Scullin <scullin@netscape.com>, 18 Nov 1997.
 *
 * Contributors: Edwin Woudt <edwin@woudt.nl>
 */

package grendel.search;

import java.awt.Component;

import grendel.ui.GeneralFrame;

public class ResultsFrame extends GeneralFrame {

  public ResultsFrame(Component aComponent) {
    super("Search Results", "searchresults");
    fPanel.add(aComponent);

    restoreBounds();
    setVisible(true);
  }

  public void dispose() {
    saveBounds();

    super.dispose();
  }
}
