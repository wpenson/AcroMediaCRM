// --- DRAGGABLE UI --- //
$( ".lead-sortable, .lead-list" ).sortable({
    connectWith: ".drag-list",
    receive : function(event, ui) {
        // assume that id for rep is "rep_x"
        var rep_id = $(this).attr("id").split('_')[1];

        // assume that id for lead is "lead_x"
        var lead_id = (ui.item[0].id).split('_')[1];

        $.ajax({
            url: "/acrocrm_leads/assign_lead/"+ lead_id + "/" + rep_id,
            success: function(result) {
                if (($('#rep_' + rep_id + ' .no-assigned-leads')).length > 0) {
                    $('#rep_' + rep_id + ' .no-assigned-leads').remove();
                }
            }
        });

        return true;
    }
}).disableSelection();

$( ".accordion" ).accordion({
    header: "h4",
    collapsible: true,
    active: false
});

function createHubspotContact(lead_id, element) {
    $.ajax({
        url: "/acrocrm_hubspot_integration/create_contact/" + lead_id,
        success: function (data) {
            var message = data.trim();
            $('#message-container').remove();
            if (message == "success") {
                var prefix = '<div id="message-container" class="row"><div class="col-lg-10 col-md-12"><div class="alert alert-success">';
                var suffix = '<br></div></div></div>';
                $(prefix + 'The HubSpot contact was created successfully' + suffix).insertAfter('#header-row');
                $('#lead_' + lead_id).remove();
            } else {
                var prefix = '<div id="message-container" class="row"><div class="col-lg-10 col-md-12"><div class="alert alert-danger">';
                var suffix = '<br></div></div></div>';

                if (message == "contact_already_exists") {
                    $(prefix + "The contact you are trying to create on HubSpot already exists. " +
                        "This could be due to a duplicate email address." + suffix).insertAfter('#header-row');
                } else if (message == "email_invalid") {
                    $(prefix + "The email address of the contact you are trying to create on HubSpot is invalid. " +
                        "HubSpot has stricter email validation than AcroCRM." + suffix).insertAfter('#header-row');
                } else if (message == "email_invalid") {
                    $(prefix + "The the contact you are trying to create could not be found. " +
                        "Refresh the page and try again" + suffix).insertAfter('#header-row');
                } else {
                    $(prefix + message + suffix).insertAfter('#header-row');
                }
            }
        },
        dataType: 'text'
    });
}

function assignPriority(lead_id, element) {
    $.ajax({
        url: "/acrocrm_leads/assign_lead_priority/" + lead_id + "/" + $(element).val()
    });
}

// --- Leads Search, Edit, and Delete --- //

Drupal.behaviors.acrocrm_leads = {
    'attach': function(context) {
        $('#lead-edit-modal').on('show.bs.modal', function (event) {
            var button = $(event.relatedTarget);
            var url = button.data('url');

            var modal = $(this);
            modal.find('.modal-content').load(url);
        });

        $('#delete-lead-confirmation-modal').on('show.bs.modal', function (event) {
            var trigger_button = $(event.relatedTarget);
            var url = trigger_button.data('url');

            var modal = $(this);

            //modal.find('#deleteLeadConfirmationButton').click(function() {
            //    $.ajax({
            //        url: url + id,
            //    })
            //    .done(function() {
            //        //modal.modal('hide');
            //        //window.location.assign('overview');
            //    })
            //    .fail(function() {
            //        alert('error');
            //    });
            //});

            modal.find('#delete-lead-confirmation-button').click(function() {
                window.location.replace(url);
            });
        });

        $('#lead-search-button').click(function(event) {
            var button = $(event.currentTarget);
            var url = button.data('url');
        });

        $('.lead-filter').click(function(event) {
            var target = $(event.currentTarget);
            var url = target.closest('ul').data('url');
            var group = target.data('group');
            var value = target.data('value');
        });
    }
};