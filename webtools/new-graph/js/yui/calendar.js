/*
Copyright (c) 2006, Yahoo! Inc. All rights reserved.
Code licensed under the BSD License:
http://developer.yahoo.net/yui/license.txt
version: 0.10.0
*/
YAHOO.widget.DateMath=new function(){this.DAY="D";this.WEEK="W";this.YEAR="Y";this.MONTH="M";this.ONE_DAY_MS=1000*60*60*24;this.add=function(date,field,amount){var d=new Date(date.getTime());switch(field)
{case this.MONTH:var newMonth=date.getMonth()+amount;var years=0;if(newMonth<0){while(newMonth<0)
{newMonth+=12;years-=1;}}else if(newMonth>11){while(newMonth>11)
{newMonth-=12;years+=1;}}
d.setMonth(newMonth);d.setFullYear(date.getFullYear()+years);break;case this.DAY:d.setDate(date.getDate()+amount);break;case this.YEAR:d.setFullYear(date.getFullYear()+amount);break;case this.WEEK:d.setDate(date.getDate()+7);break;}
return d;};this.subtract=function(date,field,amount){return this.add(date,field,(amount*-1));};this.before=function(date,compareTo){var ms=compareTo.getTime();if(date.getTime()<ms){return true;}else{return false;}};this.after=function(date,compareTo){var ms=compareTo.getTime();if(date.getTime()>ms){return true;}else{return false;}};this.getJan1=function(calendarYear){return new Date(calendarYear,0,1);};this.getDayOffset=function(date,calendarYear){var beginYear=this.getJan1(calendarYear);var dayOffset=Math.ceil((date.getTime()-beginYear.getTime())/this.ONE_DAY_MS);return dayOffset;};this.getWeekNumber=function(date,calendarYear,weekStartsOn){if(!weekStartsOn){weekStartsOn=0;}
if(!calendarYear){calendarYear=date.getFullYear();}
var weekNum=-1;var jan1=this.getJan1(calendarYear);var jan1DayOfWeek=jan1.getDay();var month=date.getMonth();var day=date.getDate();var year=date.getFullYear();var dayOffset=this.getDayOffset(date,calendarYear);if(dayOffset<0&&dayOffset>=(-1*jan1DayOfWeek)){weekNum=1;}else{weekNum=1;var testDate=this.getJan1(calendarYear);while(testDate.getTime()<date.getTime()&&testDate.getFullYear()==calendarYear){weekNum+=1;testDate=this.add(testDate,this.WEEK,1);}}
return weekNum;};this.isYearOverlapWeek=function(weekBeginDate){var overlaps=false;var nextWeek=this.add(weekBeginDate,this.DAY,6);if(nextWeek.getFullYear()!=weekBeginDate.getFullYear()){overlaps=true;}
return overlaps;};this.isMonthOverlapWeek=function(weekBeginDate){var overlaps=false;var nextWeek=this.add(weekBeginDate,this.DAY,6);if(nextWeek.getMonth()!=weekBeginDate.getMonth()){overlaps=true;}
return overlaps;};this.findMonthStart=function(date){var start=new Date(date.getFullYear(),date.getMonth(),1);return start;};this.findMonthEnd=function(date){var start=this.findMonthStart(date);var nextMonth=this.add(start,this.MONTH,1);var end=this.subtract(nextMonth,this.DAY,1);return end;};this.clearTime=function(date){date.setHours(0,0,0,0);return date;};}
YAHOO.widget.Calendar_Core=function(id,containerId,monthyear,selected){if(arguments.length>0)
{this.init(id,containerId,monthyear,selected);}}
YAHOO.widget.Calendar_Core.IMG_ROOT=(window.location.href.toLowerCase().indexOf("https")==0?"https://a248.e.akamai.net/sec.yimg.com/i/":"http://us.i1.yimg.com/us.yimg.com/i/");YAHOO.widget.Calendar_Core.DATE="D";YAHOO.widget.Calendar_Core.MONTH_DAY="MD";YAHOO.widget.Calendar_Core.WEEKDAY="WD";YAHOO.widget.Calendar_Core.RANGE="R";YAHOO.widget.Calendar_Core.MONTH="M";YAHOO.widget.Calendar_Core.DISPLAY_DAYS=42;YAHOO.widget.Calendar_Core.STOP_RENDER="S";YAHOO.widget.Calendar_Core.prototype={Config:null,parent:null,index:-1,cells:null,weekHeaderCells:null,weekFooterCells:null,cellDates:null,id:null,oDomContainer:null,today:null,renderStack:null,_renderStack:null,pageDate:null,_pageDate:null,minDate:null,maxDate:null,selectedDates:null,_selectedDates:null,shellRendered:false,table:null,headerCell:null};YAHOO.widget.Calendar_Core.prototype.init=function(id,containerId,monthyear,selected){this.setupConfig();this.id=id;this.cellDates=new Array();this.cells=new Array();this.renderStack=new Array();this._renderStack=new Array();this.oDomContainer=document.getElementById(containerId);this.today=new Date();YAHOO.widget.DateMath.clearTime(this.today);var month;var year;if(monthyear)
{var aMonthYear=monthyear.split(this.Locale.DATE_FIELD_DELIMITER);month=parseInt(aMonthYear[this.Locale.MY_MONTH_POSITION-1]);year=parseInt(aMonthYear[this.Locale.MY_YEAR_POSITION-1]);}else{month=this.today.getMonth()+1;year=this.today.getFullYear();}
this.pageDate=new Date(year,month-1,1);this._pageDate=new Date(this.pageDate.getTime());if(selected)
{this.selectedDates=this._parseDates(selected);this._selectedDates=this.selectedDates.concat();}else{this.selectedDates=new Array();this._selectedDates=new Array();}
this.wireDefaultEvents();this.wireCustomEvents();};YAHOO.widget.Calendar_Core.prototype.wireDefaultEvents=function(){this.doSelectCell=function(e,cal){var cell=this;var index=cell.index;var d=cal.cellDates[index];var date=new Date(d[0],d[1]-1,d[2]);if(!cal.isDateOOM(date)&&!YAHOO.util.Dom.hasClass(cell,cal.Style.CSS_CELL_RESTRICTED)&&!YAHOO.util.Dom.hasClass(cell,cal.Style.CSS_CELL_OOB)){if(cal.Options.MULTI_SELECT){var link=cell.getElementsByTagName("A")[0];link.blur();var cellDate=cal.cellDates[index];var cellDateIndex=cal._indexOfSelectedFieldArray(cellDate);if(cellDateIndex>-1)
{cal.deselectCell(index);}else{cal.selectCell(index);}}else{var link=cell.getElementsByTagName("A")[0];link.blur()
cal.selectCell(index);}}}
this.doCellMouseOver=function(e,cal){var cell=this;var index=cell.index;var d=cal.cellDates[index];var date=new Date(d[0],d[1]-1,d[2]);if(!cal.isDateOOM(date)&&!YAHOO.util.Dom.hasClass(cell,cal.Style.CSS_CELL_RESTRICTED)&&!YAHOO.util.Dom.hasClass(cell,cal.Style.CSS_CELL_OOB)){YAHOO.widget.Calendar_Core.prependCssClass(cell,cal.Style.CSS_CELL_HOVER);}}
this.doCellMouseOut=function(e,cal){YAHOO.widget.Calendar_Core.removeCssClass(this,cal.Style.CSS_CELL_HOVER);}
this.doNextMonth=function(e,cal){cal.nextMonth();}
this.doPreviousMonth=function(e,cal){cal.previousMonth();}}
YAHOO.widget.Calendar_Core.prototype.wireCustomEvents=function(){}
YAHOO.widget.Calendar_Core.prototype.setupConfig=function(){this.Config=new Object();this.Config.Style={CSS_ROW_HEADER:"calrowhead",CSS_ROW_FOOTER:"calrowfoot",CSS_CELL:"calcell",CSS_CELL_SELECTED:"selected",CSS_CELL_RESTRICTED:"restricted",CSS_CELL_TODAY:"today",CSS_CELL_OOM:"oom",CSS_CELL_OOB:"previous",CSS_HEADER:"calheader",CSS_HEADER_TEXT:"calhead",CSS_WEEKDAY_CELL:"calweekdaycell",CSS_WEEKDAY_ROW:"calweekdayrow",CSS_FOOTER:"calfoot",CSS_CALENDAR:"calendar",CSS_BORDER:"calbordered",CSS_CONTAINER:"calcontainer",CSS_NAV_LEFT:"calnavleft",CSS_NAV_RIGHT:"calnavright",CSS_CELL_TOP:"calcelltop",CSS_CELL_LEFT:"calcellleft",CSS_CELL_RIGHT:"calcellright",CSS_CELL_BOTTOM:"calcellbottom",CSS_CELL_HOVER:"calcellhover",CSS_CELL_HIGHLIGHT1:"highlight1",CSS_CELL_HIGHLIGHT2:"highlight2",CSS_CELL_HIGHLIGHT3:"highlight3",CSS_CELL_HIGHLIGHT4:"highlight4"};this.Style=this.Config.Style;this.Config.Locale={MONTHS_SHORT:["Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"],MONTHS_LONG:["January","February","March","April","May","June","July","August","September","October","November","December"],WEEKDAYS_1CHAR:["S","M","T","W","T","F","S"],WEEKDAYS_SHORT:["Su","Mo","Tu","We","Th","Fr","Sa"],WEEKDAYS_MEDIUM:["Sun","Mon","Tue","Wed","Thu","Fri","Sat"],WEEKDAYS_LONG:["Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"],DATE_DELIMITER:",",DATE_FIELD_DELIMITER:"/",DATE_RANGE_DELIMITER:"-",MY_MONTH_POSITION:1,MY_YEAR_POSITION:2,MD_MONTH_POSITION:1,MD_DAY_POSITION:2,MDY_MONTH_POSITION:1,MDY_DAY_POSITION:2,MDY_YEAR_POSITION:3};this.Locale=this.Config.Locale;this.Config.Options={MULTI_SELECT:false,SHOW_WEEKDAYS:true,START_WEEKDAY:0,SHOW_WEEK_HEADER:false,SHOW_WEEK_FOOTER:false,HIDE_BLANK_WEEKS:false,NAV_ARROW_LEFT:YAHOO.widget.Calendar_Core.IMG_ROOT+"us/tr/callt.gif",NAV_ARROW_RIGHT:YAHOO.widget.Calendar_Core.IMG_ROOT+"us/tr/calrt.gif"};this.Options=this.Config.Options;this.customConfig();if(!this.Options.LOCALE_MONTHS){this.Options.LOCALE_MONTHS=this.Locale.MONTHS_LONG;}
if(!this.Options.LOCALE_WEEKDAYS){this.Options.LOCALE_WEEKDAYS=this.Locale.WEEKDAYS_SHORT;}
if(this.Options.START_WEEKDAY>0)
{for(var w=0;w<this.Options.START_WEEKDAY;++w){this.Locale.WEEKDAYS_SHORT.push(this.Locale.WEEKDAYS_SHORT.shift());this.Locale.WEEKDAYS_MEDIUM.push(this.Locale.WEEKDAYS_MEDIUM.shift());this.Locale.WEEKDAYS_LONG.push(this.Locale.WEEKDAYS_LONG.shift());}}};YAHOO.widget.Calendar_Core.prototype.customConfig=function(){};YAHOO.widget.Calendar_Core.prototype.buildMonthLabel=function(){var text=this.Options.LOCALE_MONTHS[this.pageDate.getMonth()]+" "+this.pageDate.getFullYear();return text;};YAHOO.widget.Calendar_Core.prototype.buildDayLabel=function(workingDate){var day=workingDate.getDate();return day;};YAHOO.widget.Calendar_Core.prototype.buildShell=function(){this.table=document.createElement("TABLE");this.table.cellSpacing=0;YAHOO.widget.Calendar_Core.setCssClasses(this.table,[this.Style.CSS_CALENDAR]);this.table.id=this.id;this.buildShellHeader();this.buildShellBody();this.buildShellFooter();YAHOO.util.Event.addListener(window,"unload",this._unload,this);};YAHOO.widget.Calendar_Core.prototype.buildShellHeader=function(){var head=document.createElement("THEAD");var headRow=document.createElement("TR");var headerCell=document.createElement("TH");var colSpan=7;if(this.Config.Options.SHOW_WEEK_HEADER){this.weekHeaderCells=new Array();colSpan+=1;}
if(this.Config.Options.SHOW_WEEK_FOOTER){this.weekFooterCells=new Array();colSpan+=1;}
headerCell.colSpan=colSpan;YAHOO.widget.Calendar_Core.setCssClasses(headerCell,[this.Style.CSS_HEADER_TEXT]);this.headerCell=headerCell;headRow.appendChild(headerCell);head.appendChild(headRow);if(this.Options.SHOW_WEEKDAYS)
{var row=document.createElement("TR");var fillerCell;YAHOO.widget.Calendar_Core.setCssClasses(row,[this.Style.CSS_WEEKDAY_ROW]);if(this.Config.Options.SHOW_WEEK_HEADER){fillerCell=document.createElement("TH");YAHOO.widget.Calendar_Core.setCssClasses(fillerCell,[this.Style.CSS_WEEKDAY_CELL]);row.appendChild(fillerCell);}
for(var i=0;i<this.Options.LOCALE_WEEKDAYS.length;++i)
{var cell=document.createElement("TH");YAHOO.widget.Calendar_Core.setCssClasses(cell,[this.Style.CSS_WEEKDAY_CELL]);cell.innerHTML=this.Options.LOCALE_WEEKDAYS[i];row.appendChild(cell);}
if(this.Config.Options.SHOW_WEEK_FOOTER){fillerCell=document.createElement("TH");YAHOO.widget.Calendar_Core.setCssClasses(fillerCell,[this.Style.CSS_WEEKDAY_CELL]);row.appendChild(fillerCell);}
head.appendChild(row);}
this.table.appendChild(head);};YAHOO.widget.Calendar_Core.prototype.buildShellBody=function(){this.tbody=document.createElement("TBODY");for(var r=0;r<6;++r)
{var row=document.createElement("TR");for(var c=0;c<this.headerCell.colSpan;++c)
{var cell;if(this.Config.Options.SHOW_WEEK_HEADER&&c===0){cell=document.createElement("TH");this.weekHeaderCells[this.weekHeaderCells.length]=cell;}else if(this.Config.Options.SHOW_WEEK_FOOTER&&c==(this.headerCell.colSpan-1)){cell=document.createElement("TH");this.weekFooterCells[this.weekFooterCells.length]=cell;}else{cell=document.createElement("TD");this.cells[this.cells.length]=cell;YAHOO.widget.Calendar_Core.setCssClasses(cell,[this.Style.CSS_CELL]);YAHOO.util.Event.addListener(cell,"click",this.doSelectCell,this);YAHOO.util.Event.addListener(cell,"mouseover",this.doCellMouseOver,this);YAHOO.util.Event.addListener(cell,"mouseout",this.doCellMouseOut,this);}
row.appendChild(cell);}
this.tbody.appendChild(row);}
this.table.appendChild(this.tbody);};YAHOO.widget.Calendar_Core.prototype.buildShellFooter=function(){};YAHOO.widget.Calendar_Core.prototype.renderShell=function(){this.oDomContainer.appendChild(this.table);this.shellRendered=true;};YAHOO.widget.Calendar_Core.prototype.render=function(){if(!this.shellRendered)
{this.buildShell();this.renderShell();}
this.resetRenderers();this.cellDates.length=0;var workingDate=YAHOO.widget.DateMath.findMonthStart(this.pageDate);this.renderHeader();this.renderBody(workingDate);this.renderFooter();this.onRender();};YAHOO.widget.Calendar_Core.prototype.renderHeader=function(){this.headerCell.innerHTML="";var headerContainer=document.createElement("DIV");headerContainer.className=this.Style.CSS_HEADER;headerContainer.appendChild(document.createTextNode(this.buildMonthLabel()));this.headerCell.appendChild(headerContainer);};YAHOO.widget.Calendar_Core.prototype.renderBody=function(workingDate){this.preMonthDays=workingDate.getDay();if(this.Options.START_WEEKDAY>0){this.preMonthDays-=this.Options.START_WEEKDAY;}
if(this.preMonthDays<0){this.preMonthDays+=7;}
this.monthDays=YAHOO.widget.DateMath.findMonthEnd(workingDate).getDate();this.postMonthDays=YAHOO.widget.Calendar_Core.DISPLAY_DAYS-this.preMonthDays-this.monthDays;workingDate=YAHOO.widget.DateMath.subtract(workingDate,YAHOO.widget.DateMath.DAY,this.preMonthDays);var weekRowIndex=0;for(var c=0;c<this.cells.length;++c)
{var cellRenderers=new Array();var cell=this.cells[c];this.clearElement(cell);cell.index=c;cell.id=this.id+"_cell"+c;this.cellDates[this.cellDates.length]=[workingDate.getFullYear(),workingDate.getMonth()+1,workingDate.getDate()];if(workingDate.getDay()==this.Options.START_WEEKDAY){var rowHeaderCell=null;var rowFooterCell=null;if(this.Options.SHOW_WEEK_HEADER){rowHeaderCell=this.weekHeaderCells[weekRowIndex];this.clearElement(rowHeaderCell);}
if(this.Options.SHOW_WEEK_FOOTER){rowFooterCell=this.weekFooterCells[weekRowIndex];this.clearElement(rowFooterCell);}
if(this.Options.HIDE_BLANK_WEEKS&&this.isDateOOM(workingDate)&&!YAHOO.widget.DateMath.isMonthOverlapWeek(workingDate)){continue;}else{if(rowHeaderCell){this.renderRowHeader(workingDate,rowHeaderCell);}
if(rowFooterCell){this.renderRowFooter(workingDate,rowFooterCell);}}}
var renderer=null;if(workingDate.getFullYear()==this.today.getFullYear()&&workingDate.getMonth()==this.today.getMonth()&&workingDate.getDate()==this.today.getDate())
{cellRenderers[cellRenderers.length]=this.renderCellStyleToday;}
if(this.isDateOOM(workingDate))
{cellRenderers[cellRenderers.length]=this.renderCellNotThisMonth;}else{for(var r=0;r<this.renderStack.length;++r)
{var rArray=this.renderStack[r];var type=rArray[0];var month;var day;var year;switch(type){case YAHOO.widget.Calendar_Core.DATE:month=rArray[1][1];day=rArray[1][2];year=rArray[1][0];if(workingDate.getMonth()+1==month&&workingDate.getDate()==day&&workingDate.getFullYear()==year)
{renderer=rArray[2];this.renderStack.splice(r,1);}
break;case YAHOO.widget.Calendar_Core.MONTH_DAY:month=rArray[1][0];day=rArray[1][1];if(workingDate.getMonth()+1==month&&workingDate.getDate()==day)
{renderer=rArray[2];this.renderStack.splice(r,1);}
break;case YAHOO.widget.Calendar_Core.RANGE:var date1=rArray[1][0];var date2=rArray[1][1];var d1month=date1[1];var d1day=date1[2];var d1year=date1[0];var d1=new Date(d1year,d1month-1,d1day);var d2month=date2[1];var d2day=date2[2];var d2year=date2[0];var d2=new Date(d2year,d2month-1,d2day);if(workingDate.getTime()>=d1.getTime()&&workingDate.getTime()<=d2.getTime())
{renderer=rArray[2];if(workingDate.getTime()==d2.getTime()){this.renderStack.splice(r,1);}}
break;case YAHOO.widget.Calendar_Core.WEEKDAY:var weekday=rArray[1][0];if(workingDate.getDay()+1==weekday)
{renderer=rArray[2];}
break;case YAHOO.widget.Calendar_Core.MONTH:month=rArray[1][0];if(workingDate.getMonth()+1==month)
{renderer=rArray[2];}
break;}
if(renderer){cellRenderers[cellRenderers.length]=renderer;}}}
if(this._indexOfSelectedFieldArray([workingDate.getFullYear(),workingDate.getMonth()+1,workingDate.getDate()])>-1)
{cellRenderers[cellRenderers.length]=this.renderCellStyleSelected;}
if(this.minDate)
{this.minDate=YAHOO.widget.DateMath.clearTime(this.minDate);}
if(this.maxDate)
{this.maxDate=YAHOO.widget.DateMath.clearTime(this.maxDate);}
if((this.minDate&&(workingDate.getTime()<this.minDate.getTime()))||(this.maxDate&&(workingDate.getTime()>this.maxDate.getTime()))){cellRenderers[cellRenderers.length]=this.renderOutOfBoundsDate;}else{cellRenderers[cellRenderers.length]=this.renderCellDefault;}
for(var x=0;x<cellRenderers.length;++x)
{var ren=cellRenderers[x];if(ren.call(this,workingDate,cell)==YAHOO.widget.Calendar_Core.STOP_RENDER){break;}}
workingDate=YAHOO.widget.DateMath.add(workingDate,YAHOO.widget.DateMath.DAY,1);if(workingDate.getDay()==this.Options.START_WEEKDAY){weekRowIndex+=1;}
YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL);if(c>=0&&c<=6){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_TOP);}
if((c%7)==0){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_LEFT);}
if(((c+1)%7)==0){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_RIGHT);}
var postDays=this.postMonthDays;if(postDays>=7&&this.Options.HIDE_BLANK_WEEKS){var blankWeeks=Math.floor(postDays/7);for(var p=0;p<blankWeeks;++p){postDays-=7;}}
if(c>=((this.preMonthDays+postDays+this.monthDays)-7)){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_BOTTOM);}}};YAHOO.widget.Calendar_Core.prototype.renderFooter=function(){};YAHOO.widget.Calendar_Core.prototype._unload=function(e,cal){for(var c in cal.cells){c=null;}
cal.cells=null;cal.tbody=null;cal.oDomContainer=null;cal.table=null;cal.headerCell=null;cal=null;};YAHOO.widget.Calendar_Core.prototype.renderOutOfBoundsDate=function(workingDate,cell){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_OOB);cell.innerHTML=workingDate.getDate();return YAHOO.widget.Calendar_Core.STOP_RENDER;}
YAHOO.widget.Calendar_Core.prototype.renderRowHeader=function(workingDate,cell){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_ROW_HEADER);var useYear=this.pageDate.getFullYear();if(!YAHOO.widget.DateMath.isYearOverlapWeek(workingDate)){useYear=workingDate.getFullYear();}
var weekNum=YAHOO.widget.DateMath.getWeekNumber(workingDate,useYear,this.Options.START_WEEKDAY);cell.innerHTML=weekNum;if(this.isDateOOM(workingDate)&&!YAHOO.widget.DateMath.isMonthOverlapWeek(workingDate)){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_OOM);}};YAHOO.widget.Calendar_Core.prototype.renderRowFooter=function(workingDate,cell){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_ROW_FOOTER);if(this.isDateOOM(workingDate)&&!YAHOO.widget.DateMath.isMonthOverlapWeek(workingDate)){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_OOM);}};YAHOO.widget.Calendar_Core.prototype.renderCellDefault=function(workingDate,cell){cell.innerHTML="";var link=document.createElement("a");link.href="javascript:void(null);";link.name=this.id+"__"+workingDate.getFullYear()+"_"+(workingDate.getMonth()+1)+"_"+workingDate.getDate();link.appendChild(document.createTextNode(this.buildDayLabel(workingDate)));cell.appendChild(link);};YAHOO.widget.Calendar_Core.prototype.renderCellStyleHighlight1=function(workingDate,cell){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_HIGHLIGHT1);};YAHOO.widget.Calendar_Core.prototype.renderCellStyleHighlight2=function(workingDate,cell){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_HIGHLIGHT2);};YAHOO.widget.Calendar_Core.prototype.renderCellStyleHighlight3=function(workingDate,cell){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_HIGHLIGHT3);};YAHOO.widget.Calendar_Core.prototype.renderCellStyleHighlight4=function(workingDate,cell){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_HIGHLIGHT4);};YAHOO.widget.Calendar_Core.prototype.renderCellStyleToday=function(workingDate,cell){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_TODAY);};YAHOO.widget.Calendar_Core.prototype.renderCellStyleSelected=function(workingDate,cell){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_SELECTED);};YAHOO.widget.Calendar_Core.prototype.renderCellNotThisMonth=function(workingDate,cell){YAHOO.widget.Calendar_Core.addCssClass(cell,this.Style.CSS_CELL_OOM);cell.innerHTML=workingDate.getDate();return YAHOO.widget.Calendar_Core.STOP_RENDER;};YAHOO.widget.Calendar_Core.prototype.renderBodyCellRestricted=function(workingDate,cell){YAHOO.widget.Calendar_Core.setCssClasses(cell,[this.Style.CSS_CELL,this.Style.CSS_CELL_RESTRICTED]);cell.innerHTML=workingDate.getDate();return YAHOO.widget.Calendar_Core.STOP_RENDER;};YAHOO.widget.Calendar_Core.prototype.addMonths=function(count){this.pageDate=YAHOO.widget.DateMath.add(this.pageDate,YAHOO.widget.DateMath.MONTH,count);this.resetRenderers();this.onChangePage();};YAHOO.widget.Calendar_Core.prototype.subtractMonths=function(count){this.pageDate=YAHOO.widget.DateMath.subtract(this.pageDate,YAHOO.widget.DateMath.MONTH,count);this.resetRenderers();this.onChangePage();};YAHOO.widget.Calendar_Core.prototype.addYears=function(count){this.pageDate=YAHOO.widget.DateMath.add(this.pageDate,YAHOO.widget.DateMath.YEAR,count);this.resetRenderers();this.onChangePage();};YAHOO.widget.Calendar_Core.prototype.subtractYears=function(count){this.pageDate=YAHOO.widget.DateMath.subtract(this.pageDate,YAHOO.widget.DateMath.YEAR,count);this.resetRenderers();this.onChangePage();};YAHOO.widget.Calendar_Core.prototype.nextMonth=function(){this.addMonths(1);};YAHOO.widget.Calendar_Core.prototype.previousMonth=function(){this.subtractMonths(1);};YAHOO.widget.Calendar_Core.prototype.nextYear=function(){this.addYears(1);};YAHOO.widget.Calendar_Core.prototype.previousYear=function(){this.subtractYears(1);};YAHOO.widget.Calendar_Core.prototype.reset=function(){this.selectedDates.length=0;this.selectedDates=this._selectedDates.concat();this.pageDate=new Date(this._pageDate.getTime());this.onReset();};YAHOO.widget.Calendar_Core.prototype.clear=function(){this.selectedDates.length=0;this.pageDate=new Date(this.today.getTime());this.onClear();};YAHOO.widget.Calendar_Core.prototype.select=function(date){this.onBeforeSelect();var aToBeSelected=this._toFieldArray(date);for(var a=0;a<aToBeSelected.length;++a)
{var toSelect=aToBeSelected[a];if(this._indexOfSelectedFieldArray(toSelect)==-1)
{this.selectedDates[this.selectedDates.length]=toSelect;}}
if(this.parent){this.parent.sync(this);}
this.onSelect();return this.getSelectedDates();};YAHOO.widget.Calendar_Core.prototype.selectCell=function(cellIndex){this.onBeforeSelect();this.cells=this.tbody.getElementsByTagName("TD");var cell=this.cells[cellIndex];var cellDate=this.cellDates[cellIndex];var dCellDate=this._toDate(cellDate);var selectDate=cellDate.concat();this.selectedDates.push(selectDate);if(this.parent){this.parent.sync(this);}
this.renderCellStyleSelected(dCellDate,cell);this.onSelect();this.doCellMouseOut.call(cell,null,this);return this.getSelectedDates();};YAHOO.widget.Calendar_Core.prototype.deselect=function(date){this.onBeforeDeselect();var aToBeSelected=this._toFieldArray(date);for(var a=0;a<aToBeSelected.length;++a)
{var toSelect=aToBeSelected[a];var index=this._indexOfSelectedFieldArray(toSelect);if(index!=-1)
{this.selectedDates.splice(index,1);}}
if(this.parent){this.parent.sync(this);}
this.onDeselect();return this.getSelectedDates();};YAHOO.widget.Calendar_Core.prototype.deselectCell=function(i){this.onBeforeDeselect();this.cells=this.tbody.getElementsByTagName("TD");var cell=this.cells[i];var cellDate=this.cellDates[i];var cellDateIndex=this._indexOfSelectedFieldArray(cellDate);var dCellDate=this._toDate(cellDate);var selectDate=cellDate.concat();if(cellDateIndex>-1)
{if(this.pageDate.getMonth()==dCellDate.getMonth()&&this.pageDate.getFullYear()==dCellDate.getFullYear())
{YAHOO.widget.Calendar_Core.removeCssClass(cell,this.Style.CSS_CELL_SELECTED);}
this.selectedDates.splice(cellDateIndex,1);}
if(this.parent){this.parent.sync(this);}
this.onDeselect();return this.getSelectedDates();};YAHOO.widget.Calendar_Core.prototype.deselectAll=function(){this.onBeforeDeselect();var count=this.selectedDates.length;this.selectedDates.length=0;if(this.parent){this.parent.sync(this);}
if(count>0){this.onDeselect();}
return this.getSelectedDates();};YAHOO.widget.Calendar_Core.prototype._toFieldArray=function(date){var returnDate=new Array();if(date instanceof Date)
{returnDate=[[date.getFullYear(),date.getMonth()+1,date.getDate()]];}
else if(typeof date=='string')
{returnDate=this._parseDates(date);}
else if(date instanceof Array)
{for(var i=0;i<date.length;++i)
{var d=date[i];returnDate[returnDate.length]=[d.getFullYear(),d.getMonth()+1,d.getDate()];}}
return returnDate;};YAHOO.widget.Calendar_Core.prototype._toDate=function(dateFieldArray){if(dateFieldArray instanceof Date)
{return dateFieldArray;}else
{return new Date(dateFieldArray[0],dateFieldArray[1]-1,dateFieldArray[2]);}};YAHOO.widget.Calendar_Core.prototype._fieldArraysAreEqual=function(array1,array2){var match=false;if(array1[0]==array2[0]&&array1[1]==array2[1]&&array1[2]==array2[2])
{match=true;}
return match;};YAHOO.widget.Calendar_Core.prototype._indexOfSelectedFieldArray=function(find){var selected=-1;for(var s=0;s<this.selectedDates.length;++s)
{var sArray=this.selectedDates[s];if(find[0]==sArray[0]&&find[1]==sArray[1]&&find[2]==sArray[2])
{selected=s;break;}}
return selected;};YAHOO.widget.Calendar_Core.prototype.isDateOOM=function(date){var isOOM=false;if(date.getMonth()!=this.pageDate.getMonth()){isOOM=true;}
return isOOM;};YAHOO.widget.Calendar_Core.prototype.onBeforeSelect=function(){if(!this.Options.MULTI_SELECT){this.clearAllBodyCellStyles(this.Style.CSS_CELL_SELECTED);this.deselectAll();}};YAHOO.widget.Calendar_Core.prototype.onSelect=function(){};YAHOO.widget.Calendar_Core.prototype.onBeforeDeselect=function(){};YAHOO.widget.Calendar_Core.prototype.onDeselect=function(){};YAHOO.widget.Calendar_Core.prototype.onChangePage=function(){var me=this;this.renderHeader();if(this.renderProcId){clearTimeout(this.renderProcId);}
this.renderProcId=setTimeout(function(){me.render();me.renderProcId=null;},1);};YAHOO.widget.Calendar_Core.prototype.onRender=function(){};YAHOO.widget.Calendar_Core.prototype.onReset=function(){this.render();};YAHOO.widget.Calendar_Core.prototype.onClear=function(){this.render();};YAHOO.widget.Calendar_Core.prototype.validate=function(){return true;};YAHOO.widget.Calendar_Core.prototype._parseDate=function(sDate){var aDate=sDate.split(this.Locale.DATE_FIELD_DELIMITER);var rArray;if(aDate.length==2)
{rArray=[aDate[this.Locale.MD_MONTH_POSITION-1],aDate[this.Locale.MD_DAY_POSITION-1]];rArray.type=YAHOO.widget.Calendar_Core.MONTH_DAY;}else{rArray=[aDate[this.Locale.MDY_YEAR_POSITION-1],aDate[this.Locale.MDY_MONTH_POSITION-1],aDate[this.Locale.MDY_DAY_POSITION-1]];rArray.type=YAHOO.widget.Calendar_Core.DATE;}
return rArray;};YAHOO.widget.Calendar_Core.prototype._parseDates=function(sDates){var aReturn=new Array();var aDates=sDates.split(this.Locale.DATE_DELIMITER);for(var d=0;d<aDates.length;++d)
{var sDate=aDates[d];if(sDate.indexOf(this.Locale.DATE_RANGE_DELIMITER)!=-1){var aRange=sDate.split(this.Locale.DATE_RANGE_DELIMITER);var dateStart=this._parseDate(aRange[0]);var dateEnd=this._parseDate(aRange[1]);var fullRange=this._parseRange(dateStart,dateEnd);aReturn=aReturn.concat(fullRange);}else{var aDate=this._parseDate(sDate);aReturn.push(aDate);}}
return aReturn;};YAHOO.widget.Calendar_Core.prototype._parseRange=function(startDate,endDate){var dStart=new Date(startDate[0],startDate[1]-1,startDate[2]);var dCurrent=YAHOO.widget.DateMath.add(new Date(startDate[0],startDate[1]-1,startDate[2]),YAHOO.widget.DateMath.DAY,1);var dEnd=new Date(endDate[0],endDate[1]-1,endDate[2]);var results=new Array();results.push(startDate);while(dCurrent.getTime()<=dEnd.getTime())
{results.push([dCurrent.getFullYear(),dCurrent.getMonth()+1,dCurrent.getDate()]);dCurrent=YAHOO.widget.DateMath.add(dCurrent,YAHOO.widget.DateMath.DAY,1);}
return results;};YAHOO.widget.Calendar_Core.prototype.resetRenderers=function(){this.renderStack=this._renderStack.concat();};YAHOO.widget.Calendar_Core.prototype.clearElement=function(cell){cell.innerHTML="&nbsp;";cell.className="";};YAHOO.widget.Calendar_Core.prototype.addRenderer=function(sDates,fnRender){var aDates=this._parseDates(sDates);for(var i=0;i<aDates.length;++i)
{var aDate=aDates[i];if(aDate.length==2)
{if(aDate[0]instanceof Array)
{this._addRenderer(YAHOO.widget.Calendar_Core.RANGE,aDate,fnRender);}else{this._addRenderer(YAHOO.widget.Calendar_Core.MONTH_DAY,aDate,fnRender);}}else if(aDate.length==3)
{this._addRenderer(YAHOO.widget.Calendar_Core.DATE,aDate,fnRender);}}};YAHOO.widget.Calendar_Core.prototype._addRenderer=function(type,aDates,fnRender){var add=[type,aDates,fnRender];this.renderStack.unshift(add);this._renderStack=this.renderStack.concat();};YAHOO.widget.Calendar_Core.prototype.addMonthRenderer=function(month,fnRender){this._addRenderer(YAHOO.widget.Calendar_Core.MONTH,[month],fnRender);};YAHOO.widget.Calendar_Core.prototype.addWeekdayRenderer=function(weekday,fnRender){this._addRenderer(YAHOO.widget.Calendar_Core.WEEKDAY,[weekday],fnRender);};YAHOO.widget.Calendar_Core.addCssClass=function(element,style){if(element.className.length===0)
{element.className+=style;}else{element.className+=" "+style;}};YAHOO.widget.Calendar_Core.prependCssClass=function(element,style){element.className=style+" "+element.className;}
YAHOO.widget.Calendar_Core.removeCssClass=function(element,style){var aStyles=element.className.split(" ");for(var s=0;s<aStyles.length;++s)
{if(aStyles[s]==style)
{aStyles.splice(s,1);break;}}
YAHOO.widget.Calendar_Core.setCssClasses(element,aStyles);};YAHOO.widget.Calendar_Core.setCssClasses=function(element,aStyles){element.className="";var className=aStyles.join(" ");element.className=className;};YAHOO.widget.Calendar_Core.prototype.clearAllBodyCellStyles=function(style){for(var c=0;c<this.cells.length;++c)
{YAHOO.widget.Calendar_Core.removeCssClass(this.cells[c],style);}};YAHOO.widget.Calendar_Core.prototype.setMonth=function(month){this.pageDate.setMonth(month);};YAHOO.widget.Calendar_Core.prototype.setYear=function(year){this.pageDate.setFullYear(year);};YAHOO.widget.Calendar_Core.prototype.getSelectedDates=function(){var returnDates=new Array();for(var d=0;d<this.selectedDates.length;++d)
{var dateArray=this.selectedDates[d];var date=new Date(dateArray[0],dateArray[1]-1,dateArray[2]);returnDates.push(date);}
returnDates.sort();return returnDates;};YAHOO.widget.Calendar_Core._getBrowser=function()
{var ua=navigator.userAgent.toLowerCase();if(ua.indexOf('opera')!=-1)
return'opera';else if(ua.indexOf('msie')!=-1)
return'ie';else if(ua.indexOf('safari')!=-1)
return'safari';else if(ua.indexOf('gecko')!=-1)
return'gecko';else
return false;}
YAHOO.widget.Cal_Core=YAHOO.widget.Calendar_Core;YAHOO.widget.Calendar=function(id,containerId,monthyear,selected){if(arguments.length>0)
{this.init(id,containerId,monthyear,selected);}}
YAHOO.widget.Calendar.prototype=new YAHOO.widget.Calendar_Core();YAHOO.widget.Calendar.prototype.buildShell=function(){this.border=document.createElement("DIV");this.border.className=this.Style.CSS_BORDER;this.table=document.createElement("TABLE");this.table.cellSpacing=0;YAHOO.widget.Calendar_Core.setCssClasses(this.table,[this.Style.CSS_CALENDAR]);this.border.id=this.id;this.buildShellHeader();this.buildShellBody();this.buildShellFooter();};YAHOO.widget.Calendar.prototype.renderShell=function(){this.border.appendChild(this.table);this.oDomContainer.appendChild(this.border);this.shellRendered=true;};YAHOO.widget.Calendar.prototype.renderHeader=function(){this.headerCell.innerHTML="";var headerContainer=document.createElement("DIV");headerContainer.className=this.Style.CSS_HEADER;var linkLeft=document.createElement("A");linkLeft.href="javascript:"+this.id+".previousMonth()";var imgLeft=document.createElement("IMG");imgLeft.src=this.Options.NAV_ARROW_LEFT;imgLeft.className=this.Style.CSS_NAV_LEFT;linkLeft.appendChild(imgLeft);var linkRight=document.createElement("A");linkRight.href="javascript:"+this.id+".nextMonth()";var imgRight=document.createElement("IMG");imgRight.src=this.Options.NAV_ARROW_RIGHT;imgRight.className=this.Style.CSS_NAV_RIGHT;linkRight.appendChild(imgRight);headerContainer.appendChild(linkLeft);headerContainer.appendChild(document.createTextNode(this.buildMonthLabel()));headerContainer.appendChild(linkRight);this.headerCell.appendChild(headerContainer);};YAHOO.widget.Cal=YAHOO.widget.Calendar;YAHOO.widget.CalendarGroup=function(pageCount,id,containerId,monthyear,selected){if(arguments.length>0)
{this.init(pageCount,id,containerId,monthyear,selected);}}
YAHOO.widget.CalendarGroup.prototype.init=function(pageCount,id,containerId,monthyear,selected){this.id=id;this.selectedDates=new Array();this.containerId=containerId;this.pageCount=pageCount;this.pages=new Array();for(var p=0;p<pageCount;++p)
{var cal=this.constructChild(id+"_"+p,this.containerId+"_"+p,monthyear,selected);cal.parent=this;cal.index=p;cal.pageDate.setMonth(cal.pageDate.getMonth()+p);cal._pageDate=new Date(cal.pageDate.getFullYear(),cal.pageDate.getMonth(),cal.pageDate.getDate());this.pages.push(cal);}
this.doNextMonth=function(e,calGroup){calGroup.nextMonth();}
this.doPreviousMonth=function(e,calGroup){calGroup.previousMonth();}};YAHOO.widget.CalendarGroup.prototype.setChildFunction=function(fnName,fn){for(var p=0;p<this.pageCount;++p){this.pages[p][fnName]=fn;}}
YAHOO.widget.CalendarGroup.prototype.callChildFunction=function(fnName,args){for(var p=0;p<this.pageCount;++p){var page=this.pages[p];if(page[fnName]){var fn=page[fnName];fn.call(page,args);}}}
YAHOO.widget.CalendarGroup.prototype.constructChild=function(id,containerId,monthyear,selected){return new YAHOO.widget.Calendar_Core(id,containerId,monthyear,selected);};YAHOO.widget.CalendarGroup.prototype.setMonth=function(month){for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];cal.setMonth(month+p);}};YAHOO.widget.CalendarGroup.prototype.setYear=function(year){for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];if((cal.pageDate.getMonth()+1)==1&&p>0)
{year+=1;}
cal.setYear(year);}};YAHOO.widget.CalendarGroup.prototype.render=function(){for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];cal.render();}};YAHOO.widget.CalendarGroup.prototype.select=function(date){var ret;for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];ret=cal.select(date);}
return ret;};YAHOO.widget.CalendarGroup.prototype.selectCell=function(cellIndex){var ret;for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];ret=cal.selectCell(cellIndex);}
return ret;};YAHOO.widget.CalendarGroup.prototype.deselect=function(date){var ret;for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];ret=cal.deselect(date);}
return ret;};YAHOO.widget.CalendarGroup.prototype.deselectAll=function(){var ret;for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];ret=cal.deselectAll();}
return ret;};YAHOO.widget.CalendarGroup.prototype.deselectCell=function(cellIndex){for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];cal.deselectCell(cellIndex);}
return this.getSelectedDates();};YAHOO.widget.CalendarGroup.prototype.reset=function(){for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];cal.reset();}};YAHOO.widget.CalendarGroup.prototype.clear=function(){for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];cal.clear();}};YAHOO.widget.CalendarGroup.prototype.nextMonth=function(){for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];cal.nextMonth();}};YAHOO.widget.CalendarGroup.prototype.previousMonth=function(){for(var p=this.pages.length-1;p>=0;--p)
{var cal=this.pages[p];cal.previousMonth();}};YAHOO.widget.CalendarGroup.prototype.nextYear=function(){for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];cal.nextYear();}};YAHOO.widget.CalendarGroup.prototype.previousYear=function(){for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];cal.previousYear();}};YAHOO.widget.CalendarGroup.prototype.sync=function(caller){var calendar;if(caller)
{this.selectedDates=caller.selectedDates.concat();}else{var hash=new Object();var combinedDates=new Array();for(var p=0;p<this.pages.length;++p)
{calendar=this.pages[p];var values=calendar.selectedDates;for(var v=0;v<values.length;++v)
{var valueArray=values[v];hash[valueArray.toString()]=valueArray;}}
for(var val in hash)
{combinedDates[combinedDates.length]=hash[val];}
this.selectedDates=combinedDates.concat();}
for(p=0;p<this.pages.length;++p)
{calendar=this.pages[p];if(!calendar.Options.MULTI_SELECT){calendar.clearAllBodyCellStyles(calendar.Config.Style.CSS_CELL_SELECTED);}
calendar.selectedDates=this.selectedDates.concat();}
return this.getSelectedDates();};YAHOO.widget.CalendarGroup.prototype.getSelectedDates=function(){var returnDates=new Array();for(var d=0;d<this.selectedDates.length;++d)
{var dateArray=this.selectedDates[d];var date=new Date(dateArray[0],dateArray[1]-1,dateArray[2]);returnDates.push(date);}
returnDates.sort();return returnDates;};YAHOO.widget.CalendarGroup.prototype.addRenderer=function(sDates,fnRender){for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];cal.addRenderer(sDates,fnRender);}};YAHOO.widget.CalendarGroup.prototype.addMonthRenderer=function(month,fnRender){for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];cal.addMonthRenderer(month,fnRender);}};YAHOO.widget.CalendarGroup.prototype.addWeekdayRenderer=function(weekday,fnRender){for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];cal.addWeekdayRenderer(weekday,fnRender);}};YAHOO.widget.CalendarGroup.prototype.wireEvent=function(eventName,fn){for(var p=0;p<this.pages.length;++p)
{var cal=this.pages[p];cal[eventName]=fn;}};YAHOO.widget.CalGrp=YAHOO.widget.CalendarGroup;YAHOO.widget.Calendar2up_Cal=function(id,containerId,monthyear,selected){if(arguments.length>0)
{this.init(id,containerId,monthyear,selected);}}
YAHOO.widget.Calendar2up_Cal.prototype=new YAHOO.widget.Calendar_Core();YAHOO.widget.Calendar2up_Cal.prototype.renderHeader=function(){this.headerCell.innerHTML="";var headerContainer=document.createElement("DIV");headerContainer.className=this.Style.CSS_HEADER;if(this.index==0){var linkLeft=document.createElement("A");linkLeft.href="javascript:void(null)";YAHOO.util.Event.addListener(linkLeft,"click",this.parent.doPreviousMonth,this.parent);var imgLeft=document.createElement("IMG");imgLeft.src=this.Options.NAV_ARROW_LEFT;imgLeft.className=this.Style.CSS_NAV_LEFT;linkLeft.appendChild(imgLeft);headerContainer.appendChild(linkLeft);}
headerContainer.appendChild(document.createTextNode(this.buildMonthLabel()));if(this.index==1){var linkRight=document.createElement("A");linkRight.href="javascript:void(null)";YAHOO.util.Event.addListener(linkRight,"click",this.parent.doNextMonth,this.parent);var imgRight=document.createElement("IMG");imgRight.src=this.Options.NAV_ARROW_RIGHT;imgRight.className=this.Style.CSS_NAV_RIGHT;linkRight.appendChild(imgRight);headerContainer.appendChild(linkRight);}
this.headerCell.appendChild(headerContainer);};YAHOO.widget.Calendar2up=function(id,containerId,monthyear,selected){if(arguments.length>0)
{this.buildWrapper(containerId);this.init(2,id,containerId,monthyear,selected);}}
YAHOO.widget.Calendar2up.prototype=new YAHOO.widget.CalendarGroup();YAHOO.widget.Calendar2up.prototype.constructChild=function(id,containerId,monthyear,selected){var cal=new YAHOO.widget.Calendar2up_Cal(id,containerId,monthyear,selected);return cal;};YAHOO.widget.Calendar2up.prototype.buildWrapper=function(containerId){var outerContainer=document.getElementById(containerId);outerContainer.className="calcontainer";var innerContainer=document.createElement("DIV");innerContainer.className="calbordered";innerContainer.id=containerId+"_inner";var cal1Container=document.createElement("DIV");cal1Container.id=containerId+"_0";cal1Container.className="cal2up";cal1Container.style.marginRight="10px";var cal2Container=document.createElement("DIV");cal2Container.id=containerId+"_1";cal2Container.className="cal2up";outerContainer.appendChild(innerContainer);innerContainer.appendChild(cal1Container);innerContainer.appendChild(cal2Container);this.innerContainer=innerContainer;this.outerContainer=outerContainer;}
YAHOO.widget.Calendar2up.prototype.render=function(){this.renderHeader();YAHOO.widget.CalendarGroup.prototype.render.call(this);this.renderFooter();};YAHOO.widget.Calendar2up.prototype.renderHeader=function(){if(!this.title){this.title="";}
if(!this.titleDiv)
{this.titleDiv=document.createElement("DIV");if(this.title=="")
{this.titleDiv.style.display="none";}}
this.titleDiv.className="title";this.titleDiv.innerHTML=this.title;if(this.outerContainer.style.position=="absolute")
{var linkClose=document.createElement("A");linkClose.href="javascript:void(null)";YAHOO.util.Event.addListener(linkClose,"click",this.hide,this);var imgClose=document.createElement("IMG");imgClose.src=YAHOO.widget.Calendar_Core.IMG_ROOT+"us/my/bn/x_d.gif";imgClose.className="close-icon";linkClose.appendChild(imgClose);this.linkClose=linkClose;this.titleDiv.appendChild(linkClose);}
this.innerContainer.insertBefore(this.titleDiv,this.innerContainer.firstChild);}
YAHOO.widget.Calendar2up.prototype.hide=function(e,cal){if(!cal)
{cal=this;}
cal.outerContainer.style.display="none";}
YAHOO.widget.Calendar2up.prototype.renderFooter=function(){}
YAHOO.widget.Cal2up=YAHOO.widget.Calendar2up;