# -*- Mode: perl; indent-tabs-mode: nil -*-
#
# The contents of this file are subject to the Mozilla Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is the Bugzilla Bug Tracking System.
#
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation. All
# Rights Reserved.
#
# Contributor(s): Myk Melez <myk@mozilla.org>
#                 Frédéric Buclin <LpSolit@gmail.com>

use strict;

package Bugzilla::FlagType;

=head1 NAME

Bugzilla::FlagType - A module to deal with Bugzilla flag types.

=head1 SYNOPSIS

FlagType.pm provides an interface to flag types as stored in Bugzilla.
See below for more information.

=head1 NOTES

=over

=item *

Use of private functions/variables outside this module may lead to
unexpected results after an upgrade.  Please avoid using private
functions in other files/modules.  Private functions are functions
whose names start with _ or are specifically noted as being private.

=back

=cut

use Bugzilla::User;
use Bugzilla::Error;
use Bugzilla::Util;
use Bugzilla::Group;

use base qw(Bugzilla::Object);

###############################
####    Initialization     ####
###############################

=begin private

=head1 PRIVATE VARIABLES/CONSTANTS

=over

=item C<DB_COLUMNS>

basic sets of columns and tables for getting flag types from the
database.

=back

=cut

use constant DB_COLUMNS => qw(
    flagtypes.id
    flagtypes.name
    flagtypes.description
    flagtypes.cc_list
    flagtypes.target_type
    flagtypes.sortkey
    flagtypes.is_active
    flagtypes.is_requestable
    flagtypes.is_requesteeble
    flagtypes.is_multiplicable
    flagtypes.grant_group_id
    flagtypes.request_group_id
);

=pod

=over

=item C<DB_TABLE>

Which database(s) is the data coming from?

Note: when adding tables to DB_TABLE, make sure to include the separator
(i.e. words like "LEFT OUTER JOIN") before the table name, since tables take
multiple separators based on the join type, and therefore it is not possible
to join them later using a single known separator.

=back

=end private

=cut

use constant DB_TABLE => 'flagtypes';
use constant LIST_ORDER => 'flagtypes.sortkey, flagtypes.name';

###############################
####      Accessors      ######
###############################

=head2 METHODS

=over

=item C<id>

Returns the ID of the flagtype.

=item C<name>

Returns the name of the flagtype.

=item C<description>

Returns the description of the flagtype.

=item C<cc_list>

Returns the concatenated CC list for the flagtype, as a single string.

=item C<target_type>

Returns whether the flagtype applies to bugs or attachments.

=item C<is_active>

Returns whether the flagtype is active or disabled. Flags being
in a disabled flagtype are not deleted. It only prevents you from
adding new flags to it.

=item C<is_requestable>

Returns whether you can request for the given flagtype
(i.e. whether the '?' flag is available or not).

=item C<is_requesteeble>

Returns whether you can ask someone specifically or not.

=item C<is_multiplicable>

Returns whether you can have more than one flag for the given
flagtype in a given bug/attachment.

=item C<sortkey>

Returns the sortkey of the flagtype.

=back

=cut

sub id               { return $_[0]->{'id'};               }
sub name             { return $_[0]->{'name'};             }
sub description      { return $_[0]->{'description'};      }
sub cc_list          { return $_[0]->{'cc_list'};          }
sub target_type      { return $_[0]->{'target_type'} eq 'b' ? 'bug' : 'attachment'; }
sub is_active        { return $_[0]->{'is_active'};        }
sub is_requestable   { return $_[0]->{'is_requestable'};   }
sub is_requesteeble  { return $_[0]->{'is_requesteeble'};  }
sub is_multiplicable { return $_[0]->{'is_multiplicable'}; }
sub sortkey          { return $_[0]->{'sortkey'};          }

###############################
####       Methods         ####
###############################

=pod

=over

=item C<grant_group>

Returns the group (as a Bugzilla::Group object) in which a user
must be in order to grant or deny a request.

=item C<request_group>

Returns the group (as a Bugzilla::Group object) in which a user
must be in order to request or clear a flag.

=item C<flag_count>

Returns the number of flags belonging to the flagtype.

=item C<inclusions>

Return a hash of product/component IDs and names
explicitly associated with the flagtype.

=item C<exclusions>

Return a hash of product/component IDs and names
explicitly excluded from the flagtype.

=back

=cut

sub grant_group {
    my $self = shift;

    if (!defined $self->{'grant_group'} && $self->{'grant_group_id'}) {
        $self->{'grant_group'} = new Bugzilla::Group($self->{'grant_group_id'});
    }
    return $self->{'grant_group'};
}

sub request_group {
    my $self = shift;

    if (!defined $self->{'request_group'} && $self->{'request_group_id'}) {
        $self->{'request_group'} = new Bugzilla::Group($self->{'request_group_id'});
    }
    return $self->{'request_group'};
}

sub flag_count {
    my $self = shift;

    if (!defined $self->{'flag_count'}) {
        $self->{'flag_count'} =
            Bugzilla->dbh->selectrow_array('SELECT COUNT(*) FROM flags
                                            WHERE type_id = ?', undef, $self->{'id'});
    }
}

sub inclusions {
    my $self = shift;

    $self->{'inclusions'} ||= get_clusions($self->id, 'in');
    return $self->{'inclusions'};
}

sub exclusions {
    my $self = shift;

    $self->{'exclusions'} ||= get_clusions($self->id, 'ex');
    return $self->{'exclusions'};
}

######################################################################
# Public Functions
######################################################################

=pod

=head1 PUBLIC FUNCTIONS/METHODS

=over

=item C<get_clusions($id, $type)>

Return a hash of product/component IDs and names
associated with the flagtype:
$clusions{'product_name:component_name'} = "product_ID:component_ID"

=back

=cut

sub get_clusions {
    my ($id, $type) = @_;
    my $dbh = Bugzilla->dbh;

    my $list =
        $dbh->selectall_arrayref("SELECT products.id, products.name, " .
                                 "       components.id, components.name " . 
                                 "FROM flagtypes, flag${type}clusions " . 
                                 "LEFT OUTER JOIN products " .
                                 "  ON flag${type}clusions.product_id = products.id " . 
                                 "LEFT OUTER JOIN components " .
                                 "  ON flag${type}clusions.component_id = components.id " . 
                                 "WHERE flagtypes.id = ? " .
                                 " AND flag${type}clusions.type_id = flagtypes.id",
                                 undef, $id);
    my %clusions;
    foreach my $data (@$list) {
        my ($product_id, $product_name, $component_id, $component_name) = @$data;
        $product_id ||= 0;
        $product_name ||= "__Any__";
        $component_id ||= 0;
        $component_name ||= "__Any__";
        $clusions{"$product_name:$component_name"} = "$product_id:$component_id";
    }
    return \%clusions;
}

=pod

=over

=item C<match($criteria)>

Queries the database for flag types matching the given criteria
and returns a list of matching flagtype objects.

=back

=cut

sub match {
    my ($criteria) = @_;
    my $dbh = Bugzilla->dbh;

    # Depending on the criteria, we may have to append additional tables.
    my $tables = [DB_TABLE];
    my @criteria = sqlify_criteria($criteria, $tables);
    $tables = join(' ', @$tables);
    $criteria = join(' AND ', @criteria);

    my $flagtype_ids = $dbh->selectcol_arrayref("SELECT id FROM $tables WHERE $criteria");

    return Bugzilla::FlagType->new_from_list($flagtype_ids);
}

=pod

=over

=item C<count($criteria)>

Returns the total number of flag types matching the given criteria.

=back

=cut

sub count {
    my ($criteria) = @_;
    my $dbh = Bugzilla->dbh;

    # Depending on the criteria, we may have to append additional tables.
    my $tables = [DB_TABLE];
    my @criteria = sqlify_criteria($criteria, $tables);
    $tables = join(' ', @$tables);
    $criteria = join(' AND ', @criteria);

    my $count = $dbh->selectrow_array("SELECT COUNT(flagtypes.id)
                                       FROM $tables WHERE $criteria");
    return $count;
}

=pod

=over

=item C<validate($cgi, $bug_id, $attach_id)>

Get a list of flag types to validate.  Uses the "map" function
to extract flag type IDs from form field names by matching columns
whose name looks like "flag_type-nnn", where "nnn" is the ID,
and returning just the ID portion of matching field names.

If the attachment is new, it has no ID yet and $attach_id is set
to -1 to force its check anyway.

=back

=cut

sub validate {
    my ($cgi, $bug_id, $attach_id) = @_;

    my $user = Bugzilla->user;
    my $dbh = Bugzilla->dbh;

    my @ids = map(/^flag_type-(\d+)$/ ? $1 : (), $cgi->param());
  
    return unless scalar(@ids);
    
    # No flag reference should exist when changing several bugs at once.
    ThrowCodeError("flags_not_available", { type => 'b' }) unless $bug_id;

    # We don't check that these flag types are valid for
    # this bug/attachment. This check will be done later when
    # processing new flags, see Flag::FormToNewFlags().

    # All flag types have to be active
    my $inactive_flagtypes =
        $dbh->selectrow_array("SELECT 1 FROM flagtypes
                               WHERE id IN (" . join(',', @ids) . ")
                               AND is_active = 0 " .
                               $dbh->sql_limit(1));

    ThrowCodeError("flag_type_inactive") if $inactive_flagtypes;

    foreach my $id (@ids) {
        my $status = $cgi->param("flag_type-$id");
        my @requestees = $cgi->param("requestee_type-$id");
        
        # Don't bother validating types the user didn't touch.
        next if $status eq "X";

        # Make sure the flag type exists.
        my $flag_type = new Bugzilla::FlagType($id);
        $flag_type 
          || ThrowCodeError("flag_type_nonexistent", { id => $id });

        # Make sure the value of the field is a valid status.
        grep($status eq $_, qw(X + - ?))
          || ThrowCodeError("flag_status_invalid", 
                            { id => $id , status => $status });

        # Make sure the user didn't request the flag unless it's requestable.
        if ($status eq '?' && !$flag_type->is_requestable) {
            ThrowCodeError("flag_status_invalid", 
                           { id => $id , status => $status });
        }

        # Make sure the user didn't specify a requestee unless the flag
        # is specifically requestable.
        if ($status eq '?'
            && !$flag_type->is_requesteeble
            && scalar(@requestees) > 0)
        {
            ThrowCodeError("flag_requestee_disabled", { type => $flag_type });
        }

        # Make sure the user didn't enter multiple requestees for a flag
        # that can't be requested from more than one person at a time.
        if ($status eq '?'
            && !$flag_type->is_multiplicable
            && scalar(@requestees) > 1)
        {
            ThrowUserError("flag_not_multiplicable", { type => $flag_type });
        }

        # Make sure the requestees are authorized to access the bug
        # (and attachment, if this installation is using the "insider group"
        # feature and the attachment is marked private).
        if ($status eq '?' && $flag_type->is_requesteeble) {
            foreach my $login (@requestees) {
                # We know the requestee exists because we ran
                # Bugzilla::User::match_field before getting here.
                my $requestee = Bugzilla::User->new_from_login($login);
    
                # Throw an error if the user can't see the bug.
                if (!$requestee->can_see_bug($bug_id)) {
                    ThrowUserError("flag_requestee_unauthorized",
                                   { flag_type => $flag_type,
                                     requestee => $requestee,
                                     bug_id    => $bug_id,
                                     attach_id => $attach_id });
                }
                
                # Throw an error if the target is a private attachment and
                # the requestee isn't in the group of insiders who can see it.
                if ($attach_id
                    && Bugzilla->params->{"insidergroup"}
                    && $cgi->param('isprivate')
                    && !$requestee->in_group(Bugzilla->params->{"insidergroup"}))
                {
                    ThrowUserError("flag_requestee_unauthorized_attachment",
                                   { flag_type => $flag_type,
                                     requestee => $requestee,
                                     bug_id    => $bug_id,
                                     attach_id => $attach_id });
                }
            }
        }

        # Make sure the user is authorized to modify flags, see bug 180879
        # - User in the grant_group can set flags, including "+" and "-".
        next if (!$flag_type->grant_group
                 || $user->in_group_id($flag_type->grant_group->id));

        # - User in the request_group can request flags.
        next if ($status eq '?'
                 && (!$flag_type->request_group
                     || $user->in_group_id($flag_type->request_group->id)));

        # - Any other flag modification is denied
        ThrowUserError("flag_update_denied",
                        { name       => $flag_type->name,
                          status     => $status,
                          old_status => "X" });
    }
}

######################################################################
# Private Functions
######################################################################

=begin private

=head1 PRIVATE FUNCTIONS

=over

=item C<sqlify_criteria($criteria, $tables)>

Converts a hash of criteria into a list of SQL criteria.
$criteria is a reference to the criteria (field => value), 
$tables is a reference to an array of tables being accessed 
by the query.

=back

=cut

sub sqlify_criteria {
    my ($criteria, $tables) = @_;
    my $dbh = Bugzilla->dbh;

    # the generated list of SQL criteria; "1=1" is a clever way of making sure
    # there's something in the list so calling code doesn't have to check list
    # size before building a WHERE clause out of it
    my @criteria = ("1=1");

    if ($criteria->{name}) {
        my $name = $dbh->quote($criteria->{name});
        trick_taint($name); # Detaint data as we have quoted it.
        push(@criteria, "flagtypes.name = $name");
    }
    if ($criteria->{target_type}) {
        # The target type is stored in the database as a one-character string
        # ("a" for attachment and "b" for bug), but this function takes complete
        # names ("attachment" and "bug") for clarity, so we must convert them.
        my $target_type = $criteria->{target_type} eq 'bug'? 'b' : 'a';
        push(@criteria, "flagtypes.target_type = '$target_type'");
    }
    if (exists($criteria->{is_active})) {
        my $is_active = $criteria->{is_active} ? "1" : "0";
        push(@criteria, "flagtypes.is_active = $is_active");
    }
    if ($criteria->{product_id} && $criteria->{'component_id'}) {
        my $product_id = $criteria->{product_id};
        my $component_id = $criteria->{component_id};
        
        # Add inclusions to the query, which simply involves joining the table
        # by flag type ID and target product/component.
        push(@$tables, "INNER JOIN flaginclusions AS i ON flagtypes.id = i.type_id");
        push(@criteria, "(i.product_id = $product_id OR i.product_id IS NULL)");
        push(@criteria, "(i.component_id = $component_id OR i.component_id IS NULL)");
        
        # Add exclusions to the query, which is more complicated.  First of all,
        # we do a LEFT JOIN so we don't miss flag types with no exclusions.
        # Then, as with inclusions, we join on flag type ID and target product/
        # component.  However, since we want flag types that *aren't* on the
        # exclusions list, we add a WHERE criteria to use only records with
        # NULL exclusion type, i.e. without any exclusions.
        my $join_clause = "flagtypes.id = e.type_id " .
                          "AND (e.product_id = $product_id OR e.product_id IS NULL) " .
                          "AND (e.component_id = $component_id OR e.component_id IS NULL)";
        push(@$tables, "LEFT JOIN flagexclusions AS e ON ($join_clause)");
        push(@criteria, "e.type_id IS NULL");
    }
    if ($criteria->{group}) {
        my $gid = $criteria->{group};
        detaint_natural($gid);
        push(@criteria, "(flagtypes.grant_group_id = $gid " .
                        " OR flagtypes.request_group_id = $gid)");
    }
    
    return @criteria;
}

1;

=end private

=head1 SEE ALSO

=over

=item B<Bugzilla::Flags>

=back

=head1 CONTRIBUTORS

=over

=item Myk Melez <myk@mozilla.org>

=item Kevin Benton <kevin.benton@amd.com>

=item Frédéric Buclin <LpSolit@gmail.com>

=back

=cut
