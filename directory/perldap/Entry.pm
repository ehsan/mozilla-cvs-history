#############################################################################
# $Id: Entry.pm,v 1.2 1998/07/23 11:05:55 leif Exp $
#
# The contents of this file are subject to the Mozilla Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is PerlDAP. The Initial Developer of the Original
# Code is Netscape Communications Corp. and Clayton Donley. Portions
# created by Netscape are Copyright (C) Netscape Communications
# Corp., portions created by Clayton Donley are Copyright (C) Clayton
# Donley. All Rights Reserved.
#
# Contributor(s):
#
# DESCRIPTION
#    This package defines an object class to manage one single LDAP
#    entry. This entry can either be a newly created one, or one
#    retrieved from an LDAP server, using the LDAP::Connection class.
#
#############################################################################

package Mozilla::LDAP::Entry;

require Tie::Hash;
@ISA = (Tie::StdHash);


#############################################################################
# Creator, make a new tie hash instance, which will keep track of all
# changes made to the hash array. This is needed so we only update modified
# attributes.
#
sub TIEHASH
{
  my $class = shift;
  my $self = {};

  return bless $self, $class;
}


#############################################################################
# Destructor, does nothing really...
#
sub DESTROY
{
}


#############################################################################
# Store method, to keep track of changes.
#
sub STORE
{
  my ($self, $attr, $val) = ($_[0], lc $_[1], $_[2]);

  return if (($val eq "") || ($attr eq ""));

  @{$self->{"_${attr}_save"}} = @{$self->{$attr}}
    unless $self->{"_${attr}_save"};
  $self->{$attr} = $val;
  $self->{"_${attr}_modified"} = 1;

  if (($attr ne "dn") && !grep(/^$attr$/, @{$self->{_oc_order_}}))
    {
      push(@{$self->{_oc_order_}}, $attr);
    }
}


#############################################################################
# Fetch method, this is case insensitive (since LDAP is...).
#
sub FETCH
{
  my ($self, $attr) = ($_[0], lc $_[1]);

  return unless defined($self->{$attr});

  return $self->{$attr};
}


#############################################################################
# Delete method, to keep track of changes.
#
sub DELETE
{
  my ($self, $attr) = ($_[0], lc $_[1]);

  return if ($attr eq "");
  return unless defined($self->{$attr});

  $self->{"_${attr}_deleted"} = 1;
  undef $self->{$attr};
}


#############################################################################
# Mark an attribute as changed.
#
sub attrModified
{
  my ($self, $attr) = ($_[0], lc $_[1]);

  return 0 if ($attr eq "");
  return 0 unless defined($self->{$attr});

  @{$self->{"_${attr}_save"}} = @{$self->{$attr}}
    unless $self->{"_${attr}_save"};
  $self->{_self_obj_}->{"_${attr}_modified"} = 1;

  return 1;
}


#############################################################################
# Remove an attribute from the entry, basically the same as the DELETE
# method. We also make an alias for "delete" here, just in case.
#
sub remove
{
  my ($self, $attr) = ($_[0], lc $_[1]);

  return 0 if ($attr eq "");
  return 0 unless defined($self->{$attr});

  $self->{_self_obj_}->{"_${attr}_deleted"} = 1;
  undef $self->{_self_obj_}->{$attr};

  return 1;
}
*delete = \*remove;


#############################################################################
# Delete a value from an attribute, if it exists. NOTE: If it was the last
# value, we'll actually remove the entire attribute!
#
sub removeValue
{
  my ($self, $attr, $val) = ($_[0], lc $_[1], $_[2]);
  my $i = 0;
  local $_;

  return 0 if (($val eq "") || ($attr eq ""));
  return 0 unless defined($self->{$attr});

  @{$self->{"_${attr}_save"}} = @{$self->{$attr}}
    unless $self->{"_${attr}_save"};
  foreach (@{$self->{$attr}})
    {
      if ($_ eq $val)
	{
	  splice(@{$self->{$attr}}, $i, 1);
	  $action = ($self->size($attr) > 0 ? "modified" : "deleted");
	  $self->{_self_obj_}->{"_${attr}_${action}"} = 1;

	  return 1;
	}
      $i++;
    }

  return 0;
}


#############################################################################
# Add a value to an attribute. The optional third argument indicates that
# we should not enforce the uniqueness on this attibute, thus bypassing
# the test and always add the value.
#
sub addValue
{
  my $self = shift;
  my ($attr, $val, $force) = (lc $_[0], $_[1], $_[2]);
  local $_;

  return 0 if (($val eq "") || ($attr eq ""));
  if (!$force)
    {
      foreach (@{$self->{$attr}})
        {
          return 0 if ($_ eq $val);
        }
    }

  @{$self->{"_${attr}_save"}} = @{$self->{$attr}}
    unless $self->{"_${attr}_save"};

  $self->{_self_obj_}->{"_${attr}_modified"} = 1;
  push(@{$self->{$attr}}, $val);

  return 1;
}


#############################################################################
# Return TRUE or FALSE, if the attribute has the specified value. The
# optional third argument says we should do case insensitive search.
#
sub hasValue
{
  my($self, $attr, $val, $nocase) = @_;

  return 0 if (($val eq "") || ($attr eq ""));
  return 0 unless defined($self->{$attr});
  return grep(/^\Q$val\E$/i, @{$self->{$attr}}) if $nocase;
  return grep(/^\Q$val\E$/, @{$self->{$attr}});
}


#############################################################################
# Return TRUE or FALSE, if the attribute matches the specified regexp. The
# optional third argument says we should do case insensitive search.
#
sub matchValue
{
  my($self, $attr, $reg, $nocase) = @_;

  return 0 if (($reg eq "") || ($attr eq ""));
  return 0 unless defined($self->{$attr});
  return grep(/$reg/i, @{$self->{$attr}}) if $nocase;
  return grep(/$reg/, @{$self->{$attr}});
}


#############################################################################
# Set the DN of this entry.
#
sub setDN
{
  my ($self, $val) = @_;

  return 0 if ($val eq "");

  $self->{dn} = $val;

  return 1;
}


#############################################################################
# Get the DN of this entry.
#
sub getDN
{
  my ($self) = @_;

  return "$self->{dn}";
}


#############################################################################
# Modify the RDN, and update the entry accordingly
#
sub modifyRDN
{
  my ($self, $rdn) = ($_[0], lc $_[1]);

  return 0;
}


#############################################################################
#
# Return the number of elements in an attribute.
#
sub size
{
  my ($self, $attr) = ($_[0], lc $_[1]);
  my @val;

  return 0 if ($attr eq "");
  return 0 unless defined($self->{$attr});

  @val = @{$self->{$attr}};
  return $#val + 1;
}


#############################################################################
#
# Return TRUE if the attribute name is in the LDAP entry.
#
sub exists
{
  my ($self, $attr) = ($_[0], lc $_[1]);

  return 0 if ($attr eq "");
  return defined($self->{$attr});
}


#############################################################################
# Mandatory TRUE return value.
#
1;


#############################################################################
# POD documentation...
#
__END__

=head1 NAME

  Mozilla::LDAP::Entry.pm - Object class to hold one LDAP entry.

=head1 SYNOPSIS

  use Mozilla::LDAP::Connection;
  use Mozilla::LDAP::Entry;

=head1 ABSTRACT

This package is the main API for using our Perl Object Oriented LDAP
module. Even though it's certainly possible, and sometimes even necessary,
to call the native LDAP C SDK functions, we strongly recommend you use
these object classes.

It's not required to use our LdapUtils.pm package, but it's
convenient and good for portability if you use as much as you can from
that package as well. This implies using the LdapConf package as well,
even though you usually don't need to use it directly.

=head1 DESCRIPTION

First, this is not ment to be a course in how LDAP works, if you have no
experience with LDAP, I suggest you read some of the literature out
there. The LDAP Deployment Book, or the LDAP C SDK documentation are good
starting points.

There are two main object classes in this package, the primary class is to
establish and use an LDAP connection. This class tracks the LDAP
connection, it's current status, and the current search (if any). Every
time you call the B<search> method of an Ldapp object, you'll reset it's
state.

The second class is for retrieving, modifying and updating a single entry
from the LDAP server. The B<search> and B<entry> methods from the
Ldapp class returns Ldapp::Entry objects. You also have to instantiate
(and modify) a new Entry object when you want to add new entries to an
LDAP server.

To assure that changes to an entry are updated properly, we strongly
recommend you use the native methods of this object class. Even though you
can modify certain elements directly, it could cause changes not to be
committed to the LDAP server.

An entry consist of a DN, and a hash array of pointers to attribute
values. Each attribute value (except the DN) is an array, but you have to
remember the hash array in the entry stores pointers to the array, not the
array. So, to access the first CN value of an entry, you'd do

    $cn = $entry->{cn}[0];

To set the CN attribute to a completely new array of values, you'd do

    $entry->{cn} = [ "Leif Hedstrom", "The Swede" ];

As long as you remember this, and try to use native Ldapp::Entry methods,
this package will take care of most the work. Once you master this,
working with LDAP in Perl is surprisingly easy.

We already mentioned DN, which stands for Distinguished Name. Every entry
in LDAP must have a DN, and it's always guaranteed to be unique within
your LDAP database. Some typical DNs are

    uid=leif,ou=people,o=netscape.com
    cn=gene-staff,ou=mailGroup,o=netscape.com
    dc=data,dc=netscape,dc=com

There's also a term called RDN, which stands for Relative Distinguished
name. In the above examples, C<uid=leif>, C<cn=gene-staff> and C<dc=data>
are all RDNs. One particular property for a RDN is that they must be
unique within it's sub-tree. Hence, there can only be one user with
C<uid=leif> within the ou=people tree.

=head1 CREATING A NEW LDAPP OBJECT

Before you can do anything with LDAP, you'll need to instantiate at least
one Ldapp object, and connect it to an LDAP server. As you probably
guessed already, this is done with the B<new> method:

    $ldap = new Ldapp("ldap", "389", $bind, $pswd, $cert);
    die "Couldn't connect to LDAP server ldap" unless  $ldap;

The arguments are: Host name, port number, and optionally a bind-DN, it's
password, and a certificate. If there is no bind-DN, the connection will
be bound as the anonymous user. If the certificate file is specified, the
connection will be over SSL, and you should then probably connect to port
636. You have to check that the object was created properly, and take
proper actions if you couldn't get a connection.

Once a connection is established, the package will take care of the
rest. If for some reason the connection is lost, the object should
reconnect on it's own, automatically. [Note: This doesn't work
now... ]. You can use the LDAP object for any number of operations, but
since everything is currently done synchronously, you can only have one
operation active at any single time. This is a major drag, and it's
something we'll fix later (see the BUGS section).

=head1 PERFORMING LDAP SEARCHES

We assume that you are familiar with the LDAP filter syntax already, all
searches in the Ldapp object uses these filters. You should also be
familiar with LDAP URLs, and LDAP object classes. There are some of the few
things you actually must know about LDAP. Perhaps the simples filter is

    (uid=leif)

This matches all entries with the UID set to "leif". Normally that
would only match one entry, but there is no guarantee for that. To find
everyone with the name "leif", you'd instead do

    (cn=*leif*)

A more complicated search involves logic operators. To find all mail
groups owned by "leif" (or actually his DN), you could do

    (&(objectclass=mailGroup)(owner=uid=leif,ou=people,o=netscape))

The I<owner> attribute is what's called a DN attribute, so to match on it
we have to specify the entire DN in the filter above. We could of course
also do a sub string "wild card" match, but it's less efficient, and
requires indexes to perform reasonably well.

Ok, now we are prepared to actually do a real search on the LDAP server:

    $base = "o=netscape.com";
    $ldap = new Ldapp("ldap", "389", "", "");
    die "No LDAP connection" unless $ldap;

    $entry = $ldap->search($base, "sub", "(uid=leif)");
    if (! $entry) {
        # handle this event, no entries found
    } else {
        while ($entry) {
            $ldap->printentry($entry);
            $entry = $ldap->entry();
        }
    }

This is actually a poor mans implementation of the I<ldapsearch> command
line utility. The B<search> method returns an Ldapp::Entry object, which
holds the first entry from the search, if any. To get the second and
subsequent entries you call the B<entry> method, until there are no more
entries. The B<printentry> method is a convenient function, printing the
entry in the LDIF format on the standard output.

The arguments to the B<search> methods are the I<LDAP Base-DN>, the
I<scope> of the search ("base", "one" or "sub"), and the actual LDAP
I<filter>. The entry return contains the DN, and all attribute values. To
access a specific attribute value, you just have to use the hash array:

    $cn = $entry->{cn}[0];

Since many LDAP attributes can have more than one value, value of the hash
array is another array (or actually a pointer to an array). In many cases
you can just assume the value is in the first slot (indexed by [0]), but
for some attributes you have to support multiple values. To find out how
many values a specific attribute has, you'd call the B<size> method:

    $numVals = $entry->size("objectclass");

One caveat: Many LDAP attributes are case insensitive, but the methods in
the Ldapp package are not aware of this. Hence, if you compare values with
case sensitivity, you can experience weird problems. If you know an
attribute is CIS (Case Insensitive), make sure you do case insensitive
string comparisons. Unfortunately some methods in this package can't do
this, and by default will do case sensitive comparisons. We are working on
this, and in a future release some of the methods will handle this more
gracefully. As an extension (for LDAP v3.0) we could also use schema
discovery for handling this even better.

There is an alternative search method, to use LDAP URLs instead of a
filter string. This can be used to easily parse and process URLs, which is
a compact way of storing a "link" to some specific LDAP information. To
process such a search, you use the B<searchURL> method:

    $entry->searchURL("ldap:///o=netscape.com??sub?(uid=leif");

As it turns out, the B<search> method also supports LDAP URL searches. If
the search filter is a proper URL, we'll actually do an URL search
instead. This is for backward compatibility, and for ease of use.

To achieve better performance and use less memory, you can limit your
search to only retrieve certain attributes. With the LDAP URLs you specify
this as an optional parameter, and with the B<search> method you add two
more options, like

    $entry = $ldap->search($base, "sub", $filter, 0, ("mail", "cn");

The last argument specifies an array of attributes to retrieve, the fewer
the attributes, the faster the search will be. The second to last argument
is a boolean value indicating if we should retrieve only the attribute
names (and no values). In most cases you want this to be FALSE, to
retrieve both the attribute names, and all their values. To do this with
the B<searchURL> method, add a second argument, which should be 0 or 1.

=head1 MODIFYING AND CREATING NEW LDAP ENTRIES

Once you have an LDAP entry, either from a search, or created directly to
get a new empty object, you are ready to modify it. If you are creating a
new entry, the first thing to set it it's DN:

    $entry->setDN("uid=leif,ou=people,o=netscape.com");

You should not do this for an existing LDAP entry, changing the RDN (or
DN) for such an entry is currently not supported. It is however something
we'll be adding in a future release. To populate (or modify) some other
attributes, we can do

    $entry->{objectclass} = [ "top", "person", "inetOrgPerson" ];
    $entry->{cn} = [ "Leif Hedstrom" ];
    $entry->{mail} = [ "leif@netscape.com" ];

Once you are done modifying your LDAP entry, call the B<update> method
from the Ldapp object:

    $ldap->update($entry);

Or, if you are creating an entirely new object, you must call the B<add>
method:

    $ldap->add($entry);

If all comes to worse, and you have to remove an entire entry from the
LDAP server, just call the B<delete> method, like

    $ldap->delete($entry);

You can't use native Perl functions like push() and splice() on attribute
values, since they won't update the Ldap::Entry class state properly.
Instead use one of the methods provided in this class, for instance

    $ldap->addValue("cn", "The Swede");
    $ldap->removeValue("mailAlternateAddress", "leif@mcom.com");
    $ldap->remove("seeAlso");

These methods return a TRUE or FALSE value, depending on the outcome
of the operation. If there was no value to remove, or a value already
exists, we return FALSE, otherwise TRUE. To check if an attribute has a
certain value, use the B<hasValue> method, like

    if ($ldap->hasValue("mail", "leif@netscape.com")) {
        # Do something
    }

There is a similar method, B<matchValue>, which takes a regular
expression to match against, instead of the entire string. For more
information this and other methods in the Entry class, see below.
    
=head1 OBJECT CLASS Ldapp::

We have already described the fundamentals of this class earlier. This is
a summary of all available methods which you can use. Be careful not to
use any undocumented features or heaviour, since the internals in this
module is likely to change.

=head2 SEARCHING AND UPDATING ENTRIES

=over 13

=item B<new>

This creates and initialized a new LDAP connection and object. The
required arguments are host name, port number, bind DN and the bind
password. An optional argument is a certificate (public key), which causes
the LDAP connection to be established over an SSL channel. Currently we do
not support Client Authentication, so you still have to use the password.

Remember that if you use SSL, the port is (usually) 636.

=item B<search>

The B<search> method is the main entry point into the Ldapp module. It
requires at least three arguments: The Base DN, the scope, and the search
strings. Two more optional arguments can be given, the first specifies if
only attribute names should be returned (TRUE or FALSE). The second
argument is a list (array) of attributes to return.

The last option is very important for performance. If you are only
interested in say the "mail" and "mailHost" attributes, specifying this in
the search will signficantly reduce the search time.

=item B<searchURL>

This is almost identical to B<search>, except this function takes only two
arguments, an LDAP URL and an optional flag to specify if we only want the
attribute names to be returned (and no values). This function isn't very
useful, since the B<search> method will actually honor properly formed
LDAP URL's, and use it if appropriate.

=item B<entry>

This method will return the next entry from the search result, and can
therefore only be called after a succesful search has been initiated. If
there are no more entries to retrieve, it returns nothing (empty string).

=item B<update>

After modifying an Ldap::Entry entry (see below), use the B<update>
method to commit changes to the LDAP server. Only attributes that has been
changed will be updated, assuming you have used the appropriate methods in
the Entry object. For instance, do not use B<push> or B<splice> to
modify an entry, the B<update> will not recognize such changes.

=item B<delete>

This will delete the current entry, or possibly an entry as specified with
the optional argument. You can use this function to delete any entry you
like, by passing it an explicit DN. If you don't pass it this argument,
B<delete> defaults to delete the current entry, from the last call to
B<search> or B<entry> .

=item B<add>

Add a new entry to the LDAP server. Make sure you use the B<new> method
for the Ldapp::Entry object, to create a proper entry.

=item B<close>

Close the LDAP connection, and clean up the object. If you don't call this
directly, the destructor for the Ldapp:: object will do the job for you.

=back

=head2 MISCELLANEOUS METHODS

=over 13

=item B<printEntry>

Print an entry on the standard output, in a format similar to LDIF. This
is mostly useful for debugging.

=item B<isURL>

Returns TRUE or FALSE if the given argument is a properly formed URL.

=item B<getError>

Return the error code (numeric) from the last LDAP API function
call. Remember that this can only be called I<after> the successful
creation of a new Ldapp:: object.

=item B<getErrorString>

Very much like B<getError>, but return a string with a human readable
error message.

=item B<printError>

Print the last error message on standard output.

=item B<setRebindProc>

Tell the LDAP SDK to call the provided Perl function when it has to follow
referrals. The Perl function should return an array of three elements, the
new Bind DN, password and authentication method. A typical usage is

    sub rebindProc {
        return ("uid=ldapadmin", "secret", LDAP_AUTH_SIMPLE);
    }

    $ld->setRebindProc(\&rebindProc);

=item B<setDefaultRebindProc>

This is very much like the previous function, except instead of specifying
the function to use, you give it the DN, password and Auth method. Then
we'll use a default rebind procedure (internal in C) to handle the rebind
credentials. This was a solution for the Windows/NT problem/bugs we have
with rebind procedures written in Perl.

=item B<normalizeDN>

This function will remove all extraneous white spaces in the DN, and also
change all upper case characters to lower case. By default we'll normalize
the current DN (from the last search), but you can also provide an
arbitrary DN as the optional argument. We do not modify the DN in this
method, we only return a new string that is the normalized version of the
DN.

This method is useful if you need to do string comparisons on DNs, for
instance during a synchronization process.

=back

=head1 OBJECT CLASS Ldapp::Entry

This is perhaps the most important class, and you'll do most of your
interaction with this class. You are already familiar with the basics for
this object class, so this section will describe details for all methods.

=head2 CREATING A NEW ENTRY

To create a complete new entry, use the B<new> method, for instance

    $entry = new Ldapp::Entry()
    $entry->setDN("uid=leif,ou=people,dc=netscape,dc=com");
    $entry->{objectclass} = [ "top", "person", "inetOrgPerson" ];
    $entry->addValue("cn", "Leif Hedstrom");
    $entry->addValue("sn", "Hedstrom");
    $entry->addValue("givenName", "Leif");
    $entry->addValue("mail", "leif@netscape.com);
    $ldap->add($entry);

This is the minimum requirements for an LDAP entry. It must have a DN, and
it must have at least one objectclass. As it turns out, by also adding the
I<person> and I<inetOrgPerson> classes, we also must provide some more
attributes, like I<CN> and I<SN>. This is because the object classes have
these attributes marked as required, and we'd get a schema violation
without them.

=head2 ADDING AND REMOVING ATTRIBUTES AND VALUES

Most of these methods are already introduces, so we'll give a brief
summary.

=over 13

=item B<remove>

This will remove the entire attribute, including all it's values, from the
entry. Let's say you want to nuke all I<mailAlternateAddress> values:

    $entry->remove("mailAlternateAddress");

=item B<removeValue>

Remove a value from an attribute. If we found the attribute, and we
removed it, we'll return TRUE (1), otherwise FALSE (0).

=item B<addValue>

Add a value to an attribute. If the attribute value already exists, or we
couldn't add the value for any other reason, we'll return FALSE (0),
otherwise we return TRUE (1).

The optional third argument is a flag, indicating that we want to add the
attribute without checking for duplicates. This is useful if you know the
values are unique already, or if you perhaps want to have duplicates.

=item B<hasValue>

Return TRUE or FALSE if the attribute has the specified value. A typical
usage is to see if an entry is of a certain object class, e.g.

    if ($entry->hasValue("objectclass", "person", 1)) {
        # do something
    }

The (optional) third argument indicates if the string comparison should be
case insensitive or not.

=item B<matchValue>

This is very similar to B<hasValue>, except it does a regular expression
match against a full string match. It takes the same arguments, including
the optional third argument to specify case insensitive matching.

=item B<setDN>

Set the DN to the specified value. Only do this on new entries, it will
not work well if you try to do this on an existing entry. In the future,
you'll be able to use the B<modifyRDN> method.

=item B<getDN>

Return the DN for the entry.

=item B<modifyRDN>

This is currently not implemented. In my copious spare time...

=item B<size>

Return the number of values for a particular attribute. For instance

    $entry->{cn} = [ "Leif Hedstrom", "The Swede" ];
    $numVals = $entry->size("cn");

This will set C<$numVals> to two (2).

=item B<exists>

Return TRUE if the specified attribute is defined in the LDAP entry.

=back

=head1 EXAMPLES

There are plenty of examples to look at, in the is-ldap/perl directory. If
you don't have these files, get the distribution from one of the sites below. 

=head1 INSTALLATION

Installing this package is part of the Makefile supplied in the
package. To install and use this module, you'll first need to download and
install the LDAP SDK (e.g. from http://developer.netscape.com). To build
the Perl modules, just do

    perl5 Makefile.PL
    make
    make install

You'll get some questions during the first step, asking you to specify
where your LDAP SDK installed, and how to configure the package.

=head1 AVAILABILITY

The latest version of this script, and other related packages, are
available from

  http://data/software/
  ftp://ftp.ogre.com/pub/LDAP/   (external)

=head1 AUTHOR INFORMATION

Address bug reports and comments to:
leif@netscape.com

=head1 CREDITS

 

=head1 BUGS

None :). But there are lots of things to be added or changed:

=over 4

=item *

Memory usage, it's just a pig.

=item *

Add support for LDAP v3, including persistanc searches.

=item *

Clean up the hole call schema for the C SDK API. This is a mess...

=item *

Add support for modifyRDN.

=item *

We must support automatica reconnect, and timeouts.

=item *

Go through all return values, for all functions.

=item *

Use asynchronous LDAP, and support more than one concurrent LDAP
operation. This is a tough one...

=item *

Several minor bugs and issues, on the ToDo list.

=back

=head1 SEE ALSO

L<Ldapc>, L<LdapUtils> and L<Perl>

=cut
