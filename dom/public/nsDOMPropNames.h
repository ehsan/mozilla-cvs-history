/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1999 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 * Norris Boyd
 */

/* nsDOMPropNames.h -- an definition of all DOM property names used to provide 
**                     per-property security policies.
**                     AUTOMATICALLY GENERATED -- See genPropNames.pl 
*/

#ifndef nsDOMPropNames_h__
#define nsDOMPropNames_h__

#define NS_DOM_PROP_NAMES \
    "abstractview.document", \
    "appcoresmanager.add", \
    "appcoresmanager.find", \
    "appcoresmanager.remove", \
    "appcoresmanager.shutdown", \
    "appcoresmanager.startup", \
    "attr.name", \
    "attr.ownerelement", \
    "attr.specified", \
    "attr.value", \
    "barprop.visible", \
    "baseappcore.id", \
    "baseappcore.init", \
    "baseappcore.setdocumentcharset", \
    "characterdata.appenddata", \
    "characterdata.data", \
    "characterdata.deletedata", \
    "characterdata.insertdata", \
    "characterdata.length", \
    "characterdata.replacedata", \
    "characterdata.substringdata", \
    "counter.identifier", \
    "counter.liststyle", \
    "counter.separator", \
    "crmfobject.request", \
    "crypto.alert", \
    "crypto.disablerightclick", \
    "crypto.generatecrmfrequest", \
    "crypto.importusercertificates", \
    "crypto.logout", \
    "crypto.popchallengeresponse", \
    "crypto.random", \
    "crypto.signtext", \
    "crypto.version", \
    "css2properties.azimuth", \
    "css2properties.background", \
    "css2properties.backgroundattachment", \
    "css2properties.backgroundcolor", \
    "css2properties.backgroundimage", \
    "css2properties.backgroundposition", \
    "css2properties.backgroundrepeat", \
    "css2properties.border", \
    "css2properties.borderbottom", \
    "css2properties.borderbottomcolor", \
    "css2properties.borderbottomstyle", \
    "css2properties.borderbottomwidth", \
    "css2properties.bordercollapse", \
    "css2properties.bordercolor", \
    "css2properties.borderleft", \
    "css2properties.borderleftcolor", \
    "css2properties.borderleftstyle", \
    "css2properties.borderleftwidth", \
    "css2properties.borderright", \
    "css2properties.borderrightcolor", \
    "css2properties.borderrightstyle", \
    "css2properties.borderrightwidth", \
    "css2properties.borderspacing", \
    "css2properties.borderstyle", \
    "css2properties.bordertop", \
    "css2properties.bordertopcolor", \
    "css2properties.bordertopstyle", \
    "css2properties.bordertopwidth", \
    "css2properties.borderwidth", \
    "css2properties.bottom", \
    "css2properties.captionside", \
    "css2properties.clear", \
    "css2properties.clip", \
    "css2properties.color", \
    "css2properties.content", \
    "css2properties.counterincrement", \
    "css2properties.counterreset", \
    "css2properties.cssfloat", \
    "css2properties.cue", \
    "css2properties.cueafter", \
    "css2properties.cuebefore", \
    "css2properties.cursor", \
    "css2properties.direction", \
    "css2properties.display", \
    "css2properties.elevation", \
    "css2properties.emptycells", \
    "css2properties.font", \
    "css2properties.fontfamily", \
    "css2properties.fontsize", \
    "css2properties.fontsizeadjust", \
    "css2properties.fontstretch", \
    "css2properties.fontstyle", \
    "css2properties.fontvariant", \
    "css2properties.fontweight", \
    "css2properties.height", \
    "css2properties.left", \
    "css2properties.letterspacing", \
    "css2properties.lineheight", \
    "css2properties.liststyle", \
    "css2properties.liststyleimage", \
    "css2properties.liststyleposition", \
    "css2properties.liststyletype", \
    "css2properties.margin", \
    "css2properties.marginbottom", \
    "css2properties.marginleft", \
    "css2properties.marginright", \
    "css2properties.margintop", \
    "css2properties.markeroffset", \
    "css2properties.marks", \
    "css2properties.maxheight", \
    "css2properties.maxwidth", \
    "css2properties.minheight", \
    "css2properties.minwidth", \
    "css2properties.mozbinding", \
    "css2properties.mozopacity", \
    "css2properties.orphans", \
    "css2properties.outline", \
    "css2properties.outlinecolor", \
    "css2properties.outlinestyle", \
    "css2properties.outlinewidth", \
    "css2properties.overflow", \
    "css2properties.padding", \
    "css2properties.paddingbottom", \
    "css2properties.paddingleft", \
    "css2properties.paddingright", \
    "css2properties.paddingtop", \
    "css2properties.page", \
    "css2properties.pagebreakafter", \
    "css2properties.pagebreakbefore", \
    "css2properties.pagebreakinside", \
    "css2properties.pause", \
    "css2properties.pauseafter", \
    "css2properties.pausebefore", \
    "css2properties.pitch", \
    "css2properties.pitchrange", \
    "css2properties.playduring", \
    "css2properties.position", \
    "css2properties.quotes", \
    "css2properties.richness", \
    "css2properties.right", \
    "css2properties.size", \
    "css2properties.speak", \
    "css2properties.speakheader", \
    "css2properties.speaknumeral", \
    "css2properties.speakpunctuation", \
    "css2properties.speechrate", \
    "css2properties.stress", \
    "css2properties.tablelayout", \
    "css2properties.textalign", \
    "css2properties.textdecoration", \
    "css2properties.textindent", \
    "css2properties.textshadow", \
    "css2properties.texttransform", \
    "css2properties.top", \
    "css2properties.unicodebidi", \
    "css2properties.verticalalign", \
    "css2properties.visibility", \
    "css2properties.voicefamily", \
    "css2properties.volume", \
    "css2properties.whitespace", \
    "css2properties.widows", \
    "css2properties.width", \
    "css2properties.wordspacing", \
    "css2properties.zindex", \
    "cssfontfacerule.style", \
    "cssimportrule.href", \
    "cssimportrule.media", \
    "cssimportrule.stylesheet", \
    "cssmediarule.cssrules", \
    "cssmediarule.deleterule", \
    "cssmediarule.insertrule", \
    "cssmediarule.media", \
    "csspagerule.selectortext", \
    "csspagerule.style", \
    "cssprimitivevalue.getcountervalue", \
    "cssprimitivevalue.getfloatvalue", \
    "cssprimitivevalue.getrectvalue", \
    "cssprimitivevalue.getrgbcolorvalue", \
    "cssprimitivevalue.getstringvalue", \
    "cssprimitivevalue.primitivetype", \
    "cssprimitivevalue.setfloatvalue", \
    "cssprimitivevalue.setstringvalue", \
    "cssrule.csstext", \
    "cssrule.parentrule", \
    "cssrule.parentstylesheet", \
    "cssrule.type", \
    "cssrulelist.item", \
    "cssrulelist.length", \
    "cssstyledeclaration.csstext", \
    "cssstyledeclaration.getpropertycssvalue", \
    "cssstyledeclaration.getpropertypriority", \
    "cssstyledeclaration.getpropertyvalue", \
    "cssstyledeclaration.item", \
    "cssstyledeclaration.length", \
    "cssstyledeclaration.parentrule", \
    "cssstyledeclaration.removeproperty", \
    "cssstyledeclaration.setproperty", \
    "cssstylerule.selectortext", \
    "cssstylerule.style", \
    "cssstylesheet.cssrules", \
    "cssstylesheet.deleterule", \
    "cssstylesheet.insertrule", \
    "cssstylesheet.ownerrule", \
    "cssvalue.csstext", \
    "cssvalue.cssvaluetype", \
    "document.createattribute", \
    "document.createattributens", \
    "document.createcdatasection", \
    "document.createcomment", \
    "document.createdocumentfragment", \
    "document.createelement", \
    "document.createelementns", \
    "document.createentityreference", \
    "document.createprocessinginstruction", \
    "document.createtextnode", \
    "document.doctype", \
    "document.documentelement", \
    "document.getelementbyid", \
    "document.getelementsbytagname", \
    "document.getelementsbytagnamens", \
    "document.implementation", \
    "document.importnode", \
    "documentcss.getoverridestyle", \
    "documentevent.createevent", \
    "documentstyle.stylesheets", \
    "documenttype.entities", \
    "documenttype.internalsubset", \
    "documenttype.name", \
    "documenttype.notations", \
    "documenttype.publicid", \
    "documenttype.systemid", \
    "documentview.defaultview", \
    "documentxbl.addbinding", \
    "documentxbl.getanonymouselementbyattribute", \
    "documentxbl.getanonymousnodes", \
    "documentxbl.getbindingparent", \
    "documentxbl.loadbindingdocument", \
    "documentxbl.removebinding", \
    "domexception.code", \
    "domexception.message", \
    "domexception.name", \
    "domexception.result", \
    "domexception.tostring", \
    "domimplementation.createdocument", \
    "domimplementation.createdocumenttype", \
    "domimplementation.hasfeature", \
    "element.getattribute", \
    "element.getattributenode", \
    "element.getattributenodens", \
    "element.getattributens", \
    "element.getelementsbytagname", \
    "element.getelementsbytagnamens", \
    "element.hasattribute", \
    "element.hasattributens", \
    "element.removeattribute", \
    "element.removeattributenode", \
    "element.removeattributens", \
    "element.setattribute", \
    "element.setattributenode", \
    "element.setattributenodens", \
    "element.setattributens", \
    "element.tagname", \
    "entity.notationname", \
    "entity.publicid", \
    "entity.systemid", \
    "event.bubbles", \
    "event.cancelable", \
    "event.currenttarget", \
    "event.eventphase", \
    "event.initevent", \
    "event.originaltarget", \
    "event.preventbubble", \
    "event.preventcapture", \
    "event.preventdefault", \
    "event.stoppropagation", \
    "event.target", \
    "event.timestamp", \
    "event.type", \
    "eventtarget.addeventlistener", \
    "eventtarget.dispatchevent", \
    "eventtarget.removeeventlistener", \
    "history.back", \
    "history.current", \
    "history.forward", \
    "history.go", \
    "history.item", \
    "history.length", \
    "history.next", \
    "history.previous", \
    "htmlanchorelement.accesskey", \
    "htmlanchorelement.blur", \
    "htmlanchorelement.charset", \
    "htmlanchorelement.coords", \
    "htmlanchorelement.focus", \
    "htmlanchorelement.href", \
    "htmlanchorelement.hreflang", \
    "htmlanchorelement.name", \
    "htmlanchorelement.rel", \
    "htmlanchorelement.rev", \
    "htmlanchorelement.shape", \
    "htmlanchorelement.tabindex", \
    "htmlanchorelement.target", \
    "htmlanchorelement.type", \
    "htmlappletelement.align", \
    "htmlappletelement.alt", \
    "htmlappletelement.archive", \
    "htmlappletelement.code", \
    "htmlappletelement.codebase", \
    "htmlappletelement.height", \
    "htmlappletelement.hspace", \
    "htmlappletelement.name", \
    "htmlappletelement.object", \
    "htmlappletelement.vspace", \
    "htmlappletelement.width", \
    "htmlareaelement.accesskey", \
    "htmlareaelement.alt", \
    "htmlareaelement.coords", \
    "htmlareaelement.href", \
    "htmlareaelement.nohref", \
    "htmlareaelement.shape", \
    "htmlareaelement.tabindex", \
    "htmlareaelement.target", \
    "htmlbaseelement.href", \
    "htmlbaseelement.target", \
    "htmlbasefontelement.color", \
    "htmlbasefontelement.face", \
    "htmlbasefontelement.size", \
    "htmlblockquoteelement.cite", \
    "htmlbodyelement.alink", \
    "htmlbodyelement.background", \
    "htmlbodyelement.bgcolor", \
    "htmlbodyelement.link", \
    "htmlbodyelement.text", \
    "htmlbodyelement.vlink", \
    "htmlbrelement.clear", \
    "htmlbuttonelement.accesskey", \
    "htmlbuttonelement.disabled", \
    "htmlbuttonelement.form", \
    "htmlbuttonelement.name", \
    "htmlbuttonelement.tabindex", \
    "htmlbuttonelement.type", \
    "htmlbuttonelement.value", \
    "htmlcollection.item", \
    "htmlcollection.length", \
    "htmlcollection.nameditem", \
    "htmldirectoryelement.compact", \
    "htmldivelement.align", \
    "htmldlistelement.compact", \
    "htmldocument.anchors", \
    "htmldocument.applets", \
    "htmldocument.body", \
    "htmldocument.close", \
    "htmldocument.cookie", \
    "htmldocument.domain", \
    "htmldocument.forms", \
    "htmldocument.getelementsbyname", \
    "htmldocument.images", \
    "htmldocument.links", \
    "htmldocument.referrer", \
    "htmldocument.title", \
    "htmldocument.url", \
    "htmlelement.classname", \
    "htmlelement.dir", \
    "htmlelement.id", \
    "htmlelement.innerhtml", \
    "htmlelement.lang", \
    "htmlelement.offsetheight", \
    "htmlelement.offsetleft", \
    "htmlelement.offsetparent", \
    "htmlelement.offsettop", \
    "htmlelement.offsetwidth", \
    "htmlelement.style", \
    "htmlelement.title", \
    "htmlembedelement.align", \
    "htmlembedelement.height", \
    "htmlembedelement.name", \
    "htmlembedelement.src", \
    "htmlembedelement.type", \
    "htmlembedelement.width", \
    "htmlfieldsetelement.form", \
    "htmlfontelement.color", \
    "htmlfontelement.face", \
    "htmlfontelement.size", \
    "htmlformcontrollist.item", \
    "htmlformcontrollist.nameditem", \
    "htmlformelement.acceptcharset", \
    "htmlformelement.action", \
    "htmlformelement.elements", \
    "htmlformelement.enctype", \
    "htmlformelement.length", \
    "htmlformelement.method", \
    "htmlformelement.name", \
    "htmlformelement.reset", \
    "htmlformelement.submit", \
    "htmlformelement.target", \
    "htmlframeelement.contentdocument", \
    "htmlframeelement.frameborder", \
    "htmlframeelement.longdesc", \
    "htmlframeelement.marginheight", \
    "htmlframeelement.marginwidth", \
    "htmlframeelement.name", \
    "htmlframeelement.noresize", \
    "htmlframeelement.scrolling", \
    "htmlframeelement.src", \
    "htmlframesetelement.cols", \
    "htmlframesetelement.rows", \
    "htmlheadelement.profile", \
    "htmlheadingelement.align", \
    "htmlhrelement.align", \
    "htmlhrelement.noshade", \
    "htmlhrelement.size", \
    "htmlhrelement.width", \
    "htmlhtmlelement.version", \
    "htmliframeelement.align", \
    "htmliframeelement.contentdocument", \
    "htmliframeelement.frameborder", \
    "htmliframeelement.height", \
    "htmliframeelement.longdesc", \
    "htmliframeelement.marginheight", \
    "htmliframeelement.marginwidth", \
    "htmliframeelement.name", \
    "htmliframeelement.scrolling", \
    "htmliframeelement.src", \
    "htmliframeelement.width", \
    "htmlimageelement.align", \
    "htmlimageelement.alt", \
    "htmlimageelement.ismap", \
    "htmlimageelement.longdesc", \
    "htmlimageelement.lowsrc", \
    "htmlimageelement.name", \
    "htmlimageelement.src", \
    "htmlimageelement.usemap", \
    "htmlinputelement.accept", \
    "htmlinputelement.accesskey", \
    "htmlinputelement.align", \
    "htmlinputelement.alt", \
    "htmlinputelement.blur", \
    "htmlinputelement.checked", \
    "htmlinputelement.click", \
    "htmlinputelement.controllers", \
    "htmlinputelement.defaultchecked", \
    "htmlinputelement.defaultvalue", \
    "htmlinputelement.disabled", \
    "htmlinputelement.focus", \
    "htmlinputelement.form", \
    "htmlinputelement.maxlength", \
    "htmlinputelement.name", \
    "htmlinputelement.readonly", \
    "htmlinputelement.select", \
    "htmlinputelement.size", \
    "htmlinputelement.src", \
    "htmlinputelement.tabindex", \
    "htmlinputelement.type", \
    "htmlinputelement.usemap", \
    "htmlinputelement.value", \
    "htmlisindexelement.form", \
    "htmlisindexelement.prompt", \
    "htmllabelelement.accesskey", \
    "htmllabelelement.form", \
    "htmllabelelement.htmlfor", \
    "htmllayerelement.background", \
    "htmllayerelement.bgcolor", \
    "htmllayerelement.document", \
    "htmllayerelement.left", \
    "htmllayerelement.name", \
    "htmllayerelement.top", \
    "htmllayerelement.visibility", \
    "htmllayerelement.zindex", \
    "htmllegendelement.accesskey", \
    "htmllegendelement.align", \
    "htmllegendelement.form", \
    "htmllielement.type", \
    "htmllielement.value", \
    "htmllinkelement.charset", \
    "htmllinkelement.disabled", \
    "htmllinkelement.href", \
    "htmllinkelement.hreflang", \
    "htmllinkelement.media", \
    "htmllinkelement.rel", \
    "htmllinkelement.rev", \
    "htmllinkelement.target", \
    "htmllinkelement.type", \
    "htmlmapelement.areas", \
    "htmlmapelement.name", \
    "htmlmenuelement.compact", \
    "htmlmetaelement.content", \
    "htmlmetaelement.httpequiv", \
    "htmlmetaelement.name", \
    "htmlmetaelement.scheme", \
    "htmlmodelement.cite", \
    "htmlmodelement.datetime", \
    "htmlobjectelement.align", \
    "htmlobjectelement.archive", \
    "htmlobjectelement.border", \
    "htmlobjectelement.code", \
    "htmlobjectelement.codebase", \
    "htmlobjectelement.codetype", \
    "htmlobjectelement.contentdocument", \
    "htmlobjectelement.data", \
    "htmlobjectelement.declare", \
    "htmlobjectelement.form", \
    "htmlobjectelement.height", \
    "htmlobjectelement.hspace", \
    "htmlobjectelement.name", \
    "htmlobjectelement.standby", \
    "htmlobjectelement.tabindex", \
    "htmlobjectelement.type", \
    "htmlobjectelement.usemap", \
    "htmlobjectelement.vspace", \
    "htmlobjectelement.width", \
    "htmlolistelement.compact", \
    "htmlolistelement.start", \
    "htmlolistelement.type", \
    "htmloptgroupelement.disabled", \
    "htmloptgroupelement.label", \
    "htmloptionelement.defaultselected", \
    "htmloptionelement.disabled", \
    "htmloptionelement.form", \
    "htmloptionelement.index", \
    "htmloptionelement.label", \
    "htmloptionelement.selected", \
    "htmloptionelement.text", \
    "htmloptionelement.value", \
    "htmlparagraphelement.align", \
    "htmlparamelement.name", \
    "htmlparamelement.type", \
    "htmlparamelement.value", \
    "htmlparamelement.valuetype", \
    "htmlpreelement.width", \
    "htmlquoteelement.cite", \
    "htmlscriptelement.charset", \
    "htmlscriptelement.defer", \
    "htmlscriptelement.event", \
    "htmlscriptelement.htmlfor", \
    "htmlscriptelement.src", \
    "htmlscriptelement.text", \
    "htmlscriptelement.type", \
    "htmlselectelement.add", \
    "htmlselectelement.blur", \
    "htmlselectelement.disabled", \
    "htmlselectelement.focus", \
    "htmlselectelement.form", \
    "htmlselectelement.length", \
    "htmlselectelement.multiple", \
    "htmlselectelement.name", \
    "htmlselectelement.options", \
    "htmlselectelement.remove", \
    "htmlselectelement.selectedindex", \
    "htmlselectelement.size", \
    "htmlselectelement.tabindex", \
    "htmlselectelement.type", \
    "htmlselectelement.value", \
    "htmlstyleelement.disabled", \
    "htmlstyleelement.media", \
    "htmlstyleelement.type", \
    "htmltablecaptionelement.align", \
    "htmltablecellelement.abbr", \
    "htmltablecellelement.align", \
    "htmltablecellelement.axis", \
    "htmltablecellelement.bgcolor", \
    "htmltablecellelement.cellindex", \
    "htmltablecellelement.ch", \
    "htmltablecellelement.choff", \
    "htmltablecellelement.colspan", \
    "htmltablecellelement.headers", \
    "htmltablecellelement.height", \
    "htmltablecellelement.nowrap", \
    "htmltablecellelement.rowspan", \
    "htmltablecellelement.scope", \
    "htmltablecellelement.valign", \
    "htmltablecellelement.width", \
    "htmltablecolelement.align", \
    "htmltablecolelement.ch", \
    "htmltablecolelement.choff", \
    "htmltablecolelement.span", \
    "htmltablecolelement.valign", \
    "htmltablecolelement.width", \
    "htmltableelement.align", \
    "htmltableelement.bgcolor", \
    "htmltableelement.border", \
    "htmltableelement.caption", \
    "htmltableelement.cellpadding", \
    "htmltableelement.cellspacing", \
    "htmltableelement.createcaption", \
    "htmltableelement.createtfoot", \
    "htmltableelement.createthead", \
    "htmltableelement.deletecaption", \
    "htmltableelement.deleterow", \
    "htmltableelement.deletetfoot", \
    "htmltableelement.deletethead", \
    "htmltableelement.frame", \
    "htmltableelement.insertrow", \
    "htmltableelement.rows", \
    "htmltableelement.rules", \
    "htmltableelement.summary", \
    "htmltableelement.tbodies", \
    "htmltableelement.tfoot", \
    "htmltableelement.thead", \
    "htmltableelement.width", \
    "htmltablerowelement.align", \
    "htmltablerowelement.bgcolor", \
    "htmltablerowelement.cells", \
    "htmltablerowelement.ch", \
    "htmltablerowelement.choff", \
    "htmltablerowelement.deletecell", \
    "htmltablerowelement.insertcell", \
    "htmltablerowelement.rowindex", \
    "htmltablerowelement.sectionrowindex", \
    "htmltablerowelement.valign", \
    "htmltablesectionelement.align", \
    "htmltablesectionelement.ch", \
    "htmltablesectionelement.choff", \
    "htmltablesectionelement.deleterow", \
    "htmltablesectionelement.insertrow", \
    "htmltablesectionelement.rows", \
    "htmltablesectionelement.valign", \
    "htmltextareaelement.accesskey", \
    "htmltextareaelement.blur", \
    "htmltextareaelement.cols", \
    "htmltextareaelement.controllers", \
    "htmltextareaelement.defaultvalue", \
    "htmltextareaelement.disabled", \
    "htmltextareaelement.focus", \
    "htmltextareaelement.form", \
    "htmltextareaelement.name", \
    "htmltextareaelement.readonly", \
    "htmltextareaelement.rows", \
    "htmltextareaelement.select", \
    "htmltextareaelement.tabindex", \
    "htmltextareaelement.type", \
    "htmltextareaelement.value", \
    "htmltitleelement.text", \
    "htmlulistelement.compact", \
    "htmlulistelement.type", \
    "image.border", \
    "image.complete", \
    "image.height", \
    "image.hspace", \
    "image.lowsrc", \
    "image.naturalheight", \
    "image.naturalwidth", \
    "image.vspace", \
    "image.width", \
    "javascript.enabled", \
    "keyevent.altkey", \
    "keyevent.charcode", \
    "keyevent.ctrlkey", \
    "keyevent.initkeyevent", \
    "keyevent.keycode", \
    "keyevent.metakey", \
    "keyevent.shiftkey", \
    "linkstyle.sheet", \
    "location.assign", \
    "location.hash", \
    "location.host", \
    "location.hostname", \
    "location.href", \
    "location.pathname", \
    "location.port", \
    "location.protocol", \
    "location.search", \
    "location.tostring", \
    "medialist.append", \
    "medialist.delete", \
    "medialist.item", \
    "medialist.length", \
    "medialist.mediatext", \
    "mimetype.description", \
    "mimetype.enabledplugin", \
    "mimetype.suffixes", \
    "mimetype.type", \
    "mimetypearray.item", \
    "mimetypearray.length", \
    "mimetypearray.nameditem", \
    "mouseevent.button", \
    "mouseevent.clientx", \
    "mouseevent.clienty", \
    "mouseevent.initmouseevent", \
    "mouseevent.relatedtarget", \
    "mouseevent.screenx", \
    "mouseevent.screeny", \
    "mutationevent.attrchange", \
    "mutationevent.attrname", \
    "mutationevent.initmutationevent", \
    "mutationevent.newvalue", \
    "mutationevent.prevvalue", \
    "mutationevent.relatednode", \
    "namednodemap.getnameditem", \
    "namednodemap.getnameditemns", \
    "namednodemap.item", \
    "namednodemap.length", \
    "namednodemap.removenameditem", \
    "namednodemap.removenameditemns", \
    "namednodemap.setnameditem", \
    "namednodemap.setnameditemns", \
    "navigator.appcodename", \
    "navigator.appname", \
    "navigator.appversion", \
    "navigator.cookieenabled", \
    "navigator.javaenabled", \
    "navigator.language", \
    "navigator.mimetypes", \
    "navigator.oscpu", \
    "navigator.platform", \
    "navigator.plugins", \
    "navigator.preference", \
    "navigator.product", \
    "navigator.productsub", \
    "navigator.securitypolicy", \
    "navigator.taintenabled", \
    "navigator.useragent", \
    "navigator.vendor", \
    "navigator.vendorsub", \
    "node.appendchild", \
    "node.attributes", \
    "node.baseuri", \
    "node.childnodes", \
    "node.clonenode", \
    "node.firstchild", \
    "node.hasattributes", \
    "node.haschildnodes", \
    "node.insertbefore", \
    "node.issupported", \
    "node.lastchild", \
    "node.localname", \
    "node.namespaceuri", \
    "node.nextsibling", \
    "node.nodename", \
    "node.nodetype", \
    "node.nodevalue", \
    "node.normalize", \
    "node.ownerdocument", \
    "node.parentnode", \
    "node.prefix", \
    "node.previoussibling", \
    "node.removechild", \
    "node.replacechild", \
    "nodelist.item", \
    "nodelist.length", \
    "notation.publicid", \
    "notation.systemid", \
    "nsdocument.characterset", \
    "nsdocument.createrange", \
    "nsdocument.dir", \
    "nsdocument.getboxobjectfor", \
    "nsdocument.load", \
    "nsdocument.location", \
    "nsdocument.plugins", \
    "nsdocument.setboxobjectfor", \
    "nshtmlanchorelement.hash", \
    "nshtmlanchorelement.host", \
    "nshtmlanchorelement.hostname", \
    "nshtmlanchorelement.pathname", \
    "nshtmlanchorelement.port", \
    "nshtmlanchorelement.protocol", \
    "nshtmlanchorelement.search", \
    "nshtmlanchorelement.text", \
    "nshtmlareaelement.hash", \
    "nshtmlareaelement.host", \
    "nshtmlareaelement.hostname", \
    "nshtmlareaelement.pathname", \
    "nshtmlareaelement.port", \
    "nshtmlareaelement.protocol", \
    "nshtmlareaelement.search", \
    "nshtmlbuttonelement.blur", \
    "nshtmlbuttonelement.focus", \
    "nshtmldocument.alinkcolor", \
    "nshtmldocument.bgcolor", \
    "nshtmldocument.captureevents", \
    "nshtmldocument.clear", \
    "nshtmldocument.embeds", \
    "nshtmldocument.fgcolor", \
    "nshtmldocument.getselection", \
    "nshtmldocument.height", \
    "nshtmldocument.lastmodified", \
    "nshtmldocument.layers", \
    "nshtmldocument.linkcolor", \
    "nshtmldocument.nameditem", \
    "nshtmldocument.open", \
    "nshtmldocument.plugins", \
    "nshtmldocument.releaseevents", \
    "nshtmldocument.routeevent", \
    "nshtmldocument.vlinkcolor", \
    "nshtmldocument.width", \
    "nshtmldocument.write", \
    "nshtmldocument.writeln", \
    "nshtmlformelement.encoding", \
    "nshtmlformelement.item", \
    "nshtmlformelement.nameditem", \
    "nshtmlinputelement.controllers", \
    "nshtmlinputelement.selectionend", \
    "nshtmlinputelement.selectionstart", \
    "nshtmlinputelement.setselectionrange", \
    "nshtmlinputelement.textlength", \
    "nshtmloptioncollection.item", \
    "nshtmloptioncollection.length", \
    "nshtmloptioncollection.nameditem", \
    "nshtmloptioncollection.selectedindex", \
    "nshtmlselectelement.item", \
    "nshtmlselectelement.nameditem", \
    "nshtmltextareaelement.controllers", \
    "nslocation.reload", \
    "nslocation.replace", \
    "nsrange.comparenode", \
    "nsrange.comparepoint", \
    "nsrange.createcontextualfragment", \
    "nsrange.intersectsnode", \
    "nsrange.ispointinrange", \
    "nsrange.nsdetach", \
    "nsuievent.cancelbubble", \
    "nsuievent.getpreventdefault", \
    "nsuievent.ischar", \
    "nsuievent.layerx", \
    "nsuievent.layery", \
    "nsuievent.pagex", \
    "nsuievent.pagey", \
    "nsuievent.rangeoffset", \
    "nsuievent.rangeparent", \
    "nsuievent.which", \
    "pkcs11.addmodule", \
    "pkcs11.deletemodule", \
    "plugin.description", \
    "plugin.filename", \
    "plugin.item", \
    "plugin.length", \
    "plugin.name", \
    "plugin.nameditem", \
    "pluginarray.item", \
    "pluginarray.length", \
    "pluginarray.nameditem", \
    "pluginarray.refresh", \
    "processinginstruction.data", \
    "processinginstruction.target", \
    "range.clonecontents", \
    "range.clonerange", \
    "range.collapse", \
    "range.collapsed", \
    "range.commonancestorcontainer", \
    "range.compareboundarypoints", \
    "range.deletecontents", \
    "range.detach", \
    "range.endcontainer", \
    "range.endoffset", \
    "range.extractcontents", \
    "range.insertnode", \
    "range.selectnode", \
    "range.selectnodecontents", \
    "range.setend", \
    "range.setendafter", \
    "range.setendbefore", \
    "range.setstart", \
    "range.setstartafter", \
    "range.setstartbefore", \
    "range.startcontainer", \
    "range.startoffset", \
    "range.surroundcontents", \
    "range.tostring", \
    "rect.bottom", \
    "rect.left", \
    "rect.right", \
    "rect.top", \
    "rgbcolor.blue", \
    "rgbcolor.green", \
    "rgbcolor.red", \
    "screen.availheight", \
    "screen.availleft", \
    "screen.availtop", \
    "screen.availwidth", \
    "screen.colordepth", \
    "screen.height", \
    "screen.left", \
    "screen.pixeldepth", \
    "screen.top", \
    "screen.width", \
    "stylesheet.disabled", \
    "stylesheet.href", \
    "stylesheet.media", \
    "stylesheet.ownernode", \
    "stylesheet.parentstylesheet", \
    "stylesheet.title", \
    "stylesheet.type", \
    "stylesheetlist.item", \
    "stylesheetlist.length", \
    "text.splittext", \
    "textrange.rangeend", \
    "textrange.rangestart", \
    "textrange.rangetype", \
    "textrangelist.item", \
    "textrangelist.length", \
    "toolkitcore.closewindow", \
    "toolkitcore.showdialog", \
    "toolkitcore.showmodaldialog", \
    "toolkitcore.showwindow", \
    "toolkitcore.showwindowwithargs", \
    "uievent.detail", \
    "uievent.inituievent", \
    "uievent.view", \
    "viewcss.getcomputedstyle", \
    "window.document", \
    "window.frames", \
    "window.getselection", \
    "window.name", \
    "window.parent", \
    "window.scriptglobals", \
    "window.scrollbars", \
    "window.scrollby", \
    "window.scrollbylines", \
    "window.scrollbypages", \
    "window.scrollto", \
    "window.scrollx", \
    "window.scrolly", \
    "window.top", \
    "windowcollection.item", \
    "windowcollection.length", \
    "windowcollection.nameditem", \
    "windoweventowner.onabort", \
    "windoweventowner.onblur", \
    "windoweventowner.onchange", \
    "windoweventowner.onclick", \
    "windoweventowner.onclose", \
    "windoweventowner.ondragdrop", \
    "windoweventowner.onerror", \
    "windoweventowner.onfocus", \
    "windoweventowner.onkeydown", \
    "windoweventowner.onkeypress", \
    "windoweventowner.onkeyup", \
    "windoweventowner.onload", \
    "windoweventowner.onmousedown", \
    "windoweventowner.onmousemove", \
    "windoweventowner.onmouseout", \
    "windoweventowner.onmouseover", \
    "windoweventowner.onmouseup", \
    "windoweventowner.onpaint", \
    "windoweventowner.onreset", \
    "windoweventowner.onresize", \
    "windoweventowner.onscroll", \
    "windoweventowner.onselect", \
    "windoweventowner.onsubmit", \
    "windoweventowner.onunload", \
    "windowinternal._content", \
    "windowinternal.alert", \
    "windowinternal.back", \
    "windowinternal.blur", \
    "windowinternal.captureevents", \
    "windowinternal.clearinterval", \
    "windowinternal.cleartimeout", \
    "windowinternal.close", \
    "windowinternal.closed", \
    "windowinternal.confirm", \
    "windowinternal.controllers", \
    "windowinternal.crypto", \
    "windowinternal.defaultstatus", \
    "windowinternal.directories", \
    "windowinternal.disableexternalcapture", \
    "windowinternal.dump", \
    "windowinternal.enableexternalcapture", \
    "windowinternal.escape", \
    "windowinternal.focus", \
    "windowinternal.forward", \
    "windowinternal.getattention", \
    "windowinternal.history", \
    "windowinternal.home", \
    "windowinternal.innerheight", \
    "windowinternal.innerwidth", \
    "windowinternal.length", \
    "windowinternal.location", \
    "windowinternal.locationbar", \
    "windowinternal.menubar", \
    "windowinternal.moveby", \
    "windowinternal.moveto", \
    "windowinternal.navigator", \
    "windowinternal.open", \
    "windowinternal.opendialog", \
    "windowinternal.opener", \
    "windowinternal.outerheight", \
    "windowinternal.outerwidth", \
    "windowinternal.pagexoffset", \
    "windowinternal.pageyoffset", \
    "windowinternal.personalbar", \
    "windowinternal.pkcs11", \
    "windowinternal.print", \
    "windowinternal.prompt", \
    "windowinternal.prompter", \
    "windowinternal.releaseevents", \
    "windowinternal.resizeby", \
    "windowinternal.resizeto", \
    "windowinternal.routeevent", \
    "windowinternal.screen", \
    "windowinternal.screenx", \
    "windowinternal.screeny", \
    "windowinternal.scroll", \
    "windowinternal.self", \
    "windowinternal.setcursor", \
    "windowinternal.setinterval", \
    "windowinternal.settimeout", \
    "windowinternal.sidebar", \
    "windowinternal.sizetocontent", \
    "windowinternal.status", \
    "windowinternal.statusbar", \
    "windowinternal.stop", \
    "windowinternal.title", \
    "windowinternal.toolbar", \
    "windowinternal.unescape", \
    "windowinternal.updatecommands", \
    "windowinternal.window", \
    "xmlhttprequest.open", \
    "xulcommanddispatcher.active", \
    "xulcommanddispatcher.addcommandupdater", \
    "xulcommanddispatcher.advancefocus", \
    "xulcommanddispatcher.advancefocusintosubtree", \
    "xulcommanddispatcher.focusedelement", \
    "xulcommanddispatcher.focusedwindow", \
    "xulcommanddispatcher.getcontrollerforcommand", \
    "xulcommanddispatcher.getcontrollers", \
    "xulcommanddispatcher.removecommandupdater", \
    "xulcommanddispatcher.rewindfocus", \
    "xulcommanddispatcher.suppressfocus", \
    "xulcommanddispatcher.suppressfocusscroll", \
    "xulcommanddispatcher.updatecommands", \
    "xuldocument.commanddispatcher", \
    "xuldocument.getelementbyid", \
    "xuldocument.getelementsbyattribute", \
    "xuldocument.height", \
    "xuldocument.persist", \
    "xuldocument.popupnode", \
    "xuldocument.tooltipnode", \
    "xuldocument.width", \
    "xulelement.addbroadcastlistener", \
    "xulelement.anonymouscontent", \
    "xulelement.blur", \
    "xulelement.boxobject", \
    "xulelement.builder", \
    "xulelement.classname", \
    "xulelement.click", \
    "xulelement.controllers", \
    "xulelement.database", \
    "xulelement.docommand", \
    "xulelement.focus", \
    "xulelement.getelementsbyattribute", \
    "xulelement.id", \
    "xulelement.removebroadcastlistener", \
    "xulelement.resource", \
    "xulelement.style", \
    "xultreeelement.addcelltoselection", \
    "xultreeelement.additemtoselection", \
    "xultreeelement.clearcellselection", \
    "xultreeelement.clearitemselection", \
    "xultreeelement.currentcell", \
    "xultreeelement.currentitem", \
    "xultreeelement.invertselection", \
    "xultreeelement.removecellfromselection", \
    "xultreeelement.removeitemfromselection", \
    "xultreeelement.selectall", \
    "xultreeelement.selectcell", \
    "xultreeelement.selectcellrange", \
    "xultreeelement.selectedcells", \
    "xultreeelement.selecteditems", \
    "xultreeelement.selectitem", \
    "xultreeelement.selectitemrange", \
    "xultreeelement.suppressonselect", \
    "xultreeelement.timedselect", \
    "xultreeelement.togglecellselection", \
    "xultreeelement.toggleitemselection", \

#endif // nsDOMPropNames_h__
