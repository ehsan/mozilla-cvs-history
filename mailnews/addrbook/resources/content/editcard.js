var card;
var newCard = -1;
var editCardTitlePrefix = "Card for ";

function OnLoadNewCard()
{
	top.card = newCard;
}


function OnLoadEditCard()
{
	// look in arguments[0] for card
	if (window.arguments && window.arguments[0])
	{
		if ( window.arguments[0].card )
		{
			// keep card in global for later
			top.card = window.arguments[0].card;
			
			GetCardValues(top.card, frames["browser.editcard"].document);
			
			//top.window.setAttribute('title', editCardTitlePrefix + top.card.DisplayName);
		}
	}
}


function NewCardOKButton()
{
	var cardproperty = Components.classes["component://netscape/addressbook/cardproperty"].createInstance();
	cardproperty = cardproperty.QueryInterface(Components.interfaces.nsIAbCard);
	dump("cardproperty = " + cardproperty + "\n");

	if ( cardproperty )
	{
		SetCardValues(cardproperty, frames["browser.newcard"].document);
	
		cardproperty.AddCardToDatabase();
	}
	
	top.window.close();
}


function EditCardOKButton()
{
	SetCardValues(top.card, frames["browser.editcard"].document);
	
	// Need to commit changes here Candice.
	top.card.EditCardToDatabase();
	
	top.window.close();
}


// Move the data from the cardproperty to the dialog

function GetCardValues(cardproperty, doc)
{
	if ( cardproperty )
	{
		doc.getElementById('FirstName').value = cardproperty.FirstName;
		doc.getElementById('LastName').value = cardproperty.LastName;
		doc.getElementById('DisplayName').value = cardproperty.DisplayName;
		doc.getElementById('NickName').value = cardproperty.NickName;
		
		doc.getElementById('PrimaryEmail').value = cardproperty.PrimaryEmail;
		doc.getElementById('SecondEmail').value = cardproperty.SecondEmail;
		//doc.getElementById('SendPlainText').value = cardproperty.SendPlainText;
		
		doc.getElementById('WorkPhone').value = cardproperty.WorkPhone;
		doc.getElementById('HomePhone').value = cardproperty.HomePhone;
		doc.getElementById('FaxNumber').value = cardproperty.FaxNumber;
		doc.getElementById('PagerNumber').value = cardproperty.PagerNumber;
		doc.getElementById('CellularNumber').value = cardproperty.CellularNumber;

		doc.getElementById('HomeAddress').value = cardproperty.HomeAddress;
		doc.getElementById('HomeAddress2').value = cardproperty.HomeAddress2;
		doc.getElementById('HomeCity').value = cardproperty.HomeCity;
		doc.getElementById('HomeState').value = cardproperty.HomeState;
		doc.getElementById('HomeZipCode').value = cardproperty.HomeZipCode;
		doc.getElementById('HomeCountry').value = cardproperty.HomeCountry;

		doc.getElementById('JobTitle').value = cardproperty.JobTitle;
		doc.getElementById('Department').value = cardproperty.Department;
		doc.getElementById('Company').value = cardproperty.Company;
		doc.getElementById('WorkAddress').value = cardproperty.WorkAddress;
		doc.getElementById('WorkAddress2').value = cardproperty.WorkAddress2;
		doc.getElementById('WorkCity').value = cardproperty.WorkCity;
		doc.getElementById('WorkState').value = cardproperty.WorkState;
		doc.getElementById('WorkZipCode').value = cardproperty.WorkZipCode;
		doc.getElementById('WorkCountry').value = cardproperty.WorkCountry;

		doc.getElementById('WebPage1').value = cardproperty.WebPage1;

		doc.getElementById('Custom1').value = cardproperty.Custom1;
		doc.getElementById('Custom2').value = cardproperty.Custom2;
		doc.getElementById('Custom3').value = cardproperty.Custom3;
		doc.getElementById('Custom4').value = cardproperty.Custom4;
		doc.getElementById('Notes').value = cardproperty.Notes;
	}
}


// Move the data from the dialog to the cardproperty to be stored in the database

function SetCardValues(cardproperty, doc)
{
	if (cardproperty)
	{
		cardproperty.FirstName = doc.getElementById('FirstName').value;
		cardproperty.LastName = doc.getElementById('LastName').value;
		cardproperty.DisplayName = doc.getElementById('DisplayName').value;
		cardproperty.NickName = doc.getElementById('NickName').value;
		
		cardproperty.PrimaryEmail = doc.getElementById('PrimaryEmail').value;
		cardproperty.SecondEmail = doc.getElementById('SecondEmail').value;
		//cardproperty.SendPlainText = doc.getElementById('SendPlainText').value;
		
		cardproperty.WorkPhone = doc.getElementById('WorkPhone').value;
		cardproperty.HomePhone = doc.getElementById('HomePhone').value;
		cardproperty.FaxNumber = doc.getElementById('FaxNumber').value;
		cardproperty.PagerNumber = doc.getElementById('PagerNumber').value;
		cardproperty.CellularNumber = doc.getElementById('CellularNumber').value;

		cardproperty.HomeAddress = doc.getElementById('HomeAddress').value;
		cardproperty.HomeAddress2 = doc.getElementById('HomeAddress2').value;
		cardproperty.HomeCity = doc.getElementById('HomeCity').value;
		cardproperty.HomeState = doc.getElementById('HomeState').value;
		cardproperty.HomeZipCode = doc.getElementById('HomeZipCode').value;
		cardproperty.HomeCountry = doc.getElementById('HomeCountry').value;

		cardproperty.JobTitle = doc.getElementById('JobTitle').value;
		cardproperty.Department = doc.getElementById('Department').value;
		cardproperty.Company = doc.getElementById('Company').value;
		cardproperty.WorkAddress = doc.getElementById('WorkAddress').value;
		cardproperty.WorkAddress2 = doc.getElementById('WorkAddress2').value;
		cardproperty.WorkCity = doc.getElementById('WorkCity').value;
		cardproperty.WorkState = doc.getElementById('WorkState').value;
		cardproperty.WorkZipCode = doc.getElementById('WorkZipCode').value;
		cardproperty.WorkCountry = doc.getElementById('WorkCountry').value;

		cardproperty.WebPage1 = doc.getElementById('WebPage1').value;

		cardproperty.Custom1 = doc.getElementById('Custom1').value;
		cardproperty.Custom2 = doc.getElementById('Custom2').value;
		cardproperty.Custom3 = doc.getElementById('Custom3').value;
		cardproperty.Custom4 = doc.getElementById('Custom4').value;
		cardproperty.Notes = doc.getElementById('Notes').value;
	}
}


function NewCardCancelButton()
{
	top.window.close();
}

function EditCardCancelButton()
{
	top.window.close();
}

