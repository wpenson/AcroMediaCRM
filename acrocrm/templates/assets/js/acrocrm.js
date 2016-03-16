// --- Acrocrm Leads Module --- //
Drupal.behaviors.acrocrm_leads = {
    'attach': function (context) {
        $(document).ready(function (event) {
            $('[data-tooltip="tooltip"]').tooltip({container: 'body'});

            $(".sales-rep").accordion({
                header: "> h4",
                collapsible: true,
                active: false,
                heightStyle: "content",
                cursor: "move",
                beforeActivate: function (event, ui) {
                    ui.newHeader.addClass("sales-rep-open").removeClass("sales-rep-closed");
                    ui.oldHeader.addClass("sales-rep-closed").removeClass("sales-rep-open");
                }
            }).droppable({
                accept: ".unassigned-lead, .unapproved-lead",
                refreshPositions: true,
                hoverClass: "ui-state-highlight",
                tolerance: "pointer",
                activate: function (event, ui) {
                    $(".ui-accordion-header, .btn").css("cursor", "move");
                    $(".accordion").accordion("option", "cursor", "move");
                    $('[data-tooltip="tooltip"]').tooltip('disable');
                },
                deactivate: function (event, ui) {
                    $(".ui-accordion-header, .btn").css("cursor", "pointer");
                    $(".accordion").accordion("option", "cursor", "pointer");
                    $('[data-tooltip="tooltip"]').tooltip('enable');
                },
                drop: function (event, ui) {
                    var sales_rep = $(this);
                    var lead_draggable = $(ui.draggable);

                    sales_rep.find("> h4").removeClass("sales-rep-hover").bind('mouseenter mouseleave');

                    sales_rep.data("accordion-hovering", "false");
                    if (sales_rep.data("accordion-prev-active") === "false") {
                        sales_rep.accordion("option", "active", false); // Close accordion
                    }

                    var original_rep_id = (lead_draggable.data('assigned-rep-id') != undefined) ? lead_draggable.data('assigned-rep-id') : null;

                    if (original_rep_id != null && original_rep_id == sales_rep.data('rep-id')) {
                        return;
                    }

                    var url = lead_draggable.data("url") + 'assign_lead/';
                    var params = sales_rep.data('rep-id') + '/' + lead_draggable.data('lead-id');

                    // TODO: Show loading indicator in the sales-rep-lead-list
                    sales_rep.find(".sales-rep-lead-list").load(url + params, function (response, status, xhr) {
                        if (status == "success") {
                            if (original_rep_id == null) {
                                if ($('.lead-filter[data-group="show"][data-value="all"] .glyphicon-ok').length != 0) {
                                    loadLeadList(event);
                                    //lead_draggable.removeClass('unassigned-lead');
                                    //lead_draggable.draggable('destroy');
                                    //lead_draggable.find('h4 span').removeClass('grey-ball').addClass('yellow-ball').data('original-title', 'Assigned but Not Approved');
                                }
                                else {
                                    lead_draggable.remove();
                                }

                                var leads_list = $('#leads-list');
                                if (leads_list.find("li").length == 0) {
                                    leads_list.html('<li class="list-group-item lead-list-error-box">No Leads Found</li>');
                                }
                            }
                            else {
                                lead_draggable.remove();

                                var original_sales_rep = $('.sales-rep[data-rep-id="' + original_rep_id + '"]');
                                if (original_sales_rep.find("li").length == 0) {
                                    original_sales_rep.find('ul').html('<li class="list-group-item no-assigned-leads">No Assigned Leads</li>');
                                }
                            }

                            loadSalesRepInteractions();

                            // update priority counts for the rep that the lead was assigned to and if applicable the rep
                            // that the lead was taken from
                            var url = lead_draggable.data("url") + 'get_lead_priority/';
                            var params = lead_draggable.data('lead-id');
                            $.get(url + params, function (response, status, xhr) {
                                var message = response.trim();
                                if (message == 'low') {
                                    $('#low-' + sales_rep.data('rep-id')).html(parseInt($('#low-' + sales_rep.data('rep-id')).html()) + 1);
                                    if (original_rep_id != null) {
                                        $('#low-' + original_rep_id).html(parseInt($('#low-' + original_rep_id).html()) - 1);
                                    }
                                }
                                else if (message == 'medium') {
                                    $('#med-' + sales_rep.data('rep-id')).html(parseInt($('#med-' + sales_rep.data('rep-id')).html()) + 1);
                                    if (original_rep_id != null) {
                                        $('#med-' + original_rep_id).html(parseInt($('#med-' + original_rep_id).html()) - 1);
                                    }
                                }
                                else if (message == 'high') {
                                    $('#high-' + sales_rep.data('rep-id')).html(parseInt($('#high-' + sales_rep.data('rep-id')).html()) + 1);
                                    if (original_rep_id != null) {
                                        $('#high-' + original_rep_id).html(parseInt($('#high-' + original_rep_id).html()) - 1);
                                    }
                                }
                                else if (message == 'unassigned') {
                                    $('#unassigned-' + sales_rep.data('rep-id')).html(parseInt($('#unassigned-' + sales_rep.data('rep-id')).html()) + 1);
                                    if (original_rep_id != null) {
                                        $('#unassigned-' + original_rep_id).html(parseInt($('#unassigned-' + original_rep_id).html()) - 1);
                                    }
                                }
                                else {
                                    console.log('An error occurred when trying to get lead priority');
                                    displayAlertMsg("error", 'An error occurred when trying to get lead priority, unable ' +
                                        'to update lead priority.')
                                }
                            }, 'text');
                        }
                        else if (status == "error") {
                            event.preventDefault();
                            displayAlertMsg("error", "Unable to assign lead.");
                        }

                        // TODO: Hide the loading indicator in the sales-rep-lead-list
                    });
                },
                over: function (event, ui) {
                    var sales_rep = $(this);
                    sales_rep.find("> h4").addClass("sales-rep-hover").unbind('mouseenter mouseleave');

                    if (sales_rep.accordion("option", "active") === 0) {
                        sales_rep.data("accordion-prev-active", "true"); // Accordion was previously active
                    }
                    else {
                        sales_rep.data("accordion-hovering", "true");

                        setTimeout(function () {
                            if (sales_rep.data("accordion-hovering") === "true") {
                                sales_rep.accordion("option", "active", 0); // Expand accordion
                                sales_rep.data("accordion-prev-active", "false"); // Accordion was not previously active
                                sales_rep.data("accordion-hovering", "false");
                            }
                        }, 750);
                    }
                },
                out: function (event, ui) {
                    var sales_rep = $(this);
                    sales_rep.find("> h4").removeClass("sales-rep-hover").bind('mouseenter mouseleave');

                    sales_rep.data("accordion-hovering", "false");
                    if (sales_rep.data("accordion-prev-active") === "false") {
                        sales_rep.accordion("option", "active", false); // Close accordion
                    }
                }
            });

            $('#leads-list').droppable({
                accept: ".unapproved-lead",
                refreshPositions: true,
                hoverClass: "ui-state-highlight",
                tolerance: "pointer",
                drop: function (event, ui) {
                    var leads_list = $(this);
                    var lead_draggable = $(ui.draggable);

                    lead_draggable.hide();
                    var url = lead_draggable.data("url") + 'unassign_lead/';
                    var params = lead_draggable.data('lead-id');

                    // TODO: Show loading indicator for the leads-list

                    leads_list.load(url + params, function (response, status, xhr) {
                        if (status == "success") {
                            lead_draggable.remove();
                            loadLeadList(event); // TODO: Check for success or fail
                            var original_rep_id = (lead_draggable.data('assigned-rep-id') != undefined) ? lead_draggable.data('assigned-rep-id') : null;
                            var original_sales_rep = $('.sales-rep[data-rep-id="' + original_rep_id + '"]');

                            if (original_sales_rep.find("li").length == 0) {
                                original_sales_rep.find('ul').html('<li class="list-group-item no-assigned-leads">No Assigned Leads</li>');
                            }

                            // update priority counts for the rep that the lead was assigned to and if applicable the rep
                            // that the lead was taken from
                            var url = lead_draggable.data("url") + 'get_lead_priority/';
                            var params = lead_draggable.data('lead-id');
                            $.get(url + params,
                                function (response, status, xhr) {
                                    var message = response.trim();
                                    if (response == 'low') {
                                        $('#low-' + original_rep_id).html(parseInt($('#low-' + original_rep_id).html()) - 1);
                                    }
                                    else if (message === 'medium') {
                                        $('#med-' + original_rep_id).html(parseInt($('#med-' + original_rep_id).html()) - 1);
                                    }
                                    else if (message === 'high') {
                                        $('#high-' + original_rep_id).html(parseInt($('#high-' + original_rep_id).html()) - 1);
                                    }
                                    else if (message === 'unassigned') {
                                        $('#unassigned-' + original_rep_id).html(parseInt($('#unassigned-' + original_rep_id).html()) - 1);
                                    }
                                    else {
                                        console.log('An error occurred when trying to get lead priority');
                                        displayAlertMsg("error", 'An error occurred when trying to get lead priority, unable ' +
                                            'to update lead priority.')
                                    }
                                },
                                'text');
                        }
                        else if (status == "error") {
                            event.preventDefault();
                            lead_draggable.show();
                            displayAlertMsg("error", "Unable to assign lead.");
                        }

                        // TODO: Hide loading indicator for the leads-list
                    });
                }
            });

            loadLeadList(event);
            loadSalesRepInteractions();
        });

        function loadSalesRepInteractions() {
            $('.readmore').readmore({
                collapsedHeight: 18
            });

            $('[data-tooltip="tooltip"]').tooltip({container: 'body'});

            // TODO: Need to fix the issue when the edit or delete lead button is pressed where the click event also triggers the accordion
            // TODO: The accordion slightly jumps on close. This is caused by a margin or something and can be fixed.
            $(".sales-rep-lead").accordion({
                header: "> h5",
                collapsible: true,
                active: false,
                heightStyle: "content",
                cursor: "move",
                activate: function (event, ui) {
                    $('.readmore').readmore({
                        collapsedHeight: 18
                    });
                }
            });

            // Unapproved leads are the leads that have been approved but are not sent to hubspot, etc.
            $(".unapproved-lead").draggable({
                appendTo: "body",
                zIndex: 100,
                helper: "clone",
                revert: "invalid",
                cursor: "move",
                cursorAt: {top: 0, left: 0},
                refreshPositions: true,
                start: function (event, ui) {
                    $(this).hide();

                    var original_rep_id = ($(this).data('assigned-rep-id') != undefined) ? $(this).data('assigned-rep-id') : null;
                    var original_sales_rep = $('.sales-rep[data-rep-id="' + original_rep_id + '"]');
                    if (original_sales_rep.find("li").length <= 1) {
                        original_sales_rep.find('ul').append('<li class="list-group-item no-assigned-leads">No Assigned Leads</li>');
                    }

                    var lead_clone = $(ui.helper);
                    lead_clone.html('<h4>' + lead_clone.find("h5").text() + '</h4>');
                    lead_clone.addClass("dragging-lead");
                },
                stop: function (event, ui) {
                    $(this).show();

                    var original_rep_id = ($(this).data('assigned-rep-id') != undefined) ? $(this).data('assigned-rep-id') : null;
                    var original_sales_rep = $('.sales-rep[data-rep-id="' + original_rep_id + '"]');
                    original_sales_rep.find('.no-assigned-leads').remove();
                }
            });
        }

        function loadLeadsListInteractions() {
            $('.readmore').readmore({
                collapsedHeight: 18
            });

            $(".unassigned-lead").draggable({
                appendTo: "body",
                zIndex: 100,
                helper: "clone",
                revert: "invalid",
                cursor: "move",
                cursorAt: {top: 0, left: 0},
                refreshPositions: true,
                start: function (event, ui) {
                    if ($('.lead-filter[data-group="show"][data-value="all"] .glyphicon-ok').length == 0) {
                        $(this).hide();
                    }

                    var leads_list = $('#leads-list');
                    if (leads_list.find("li").length <= 1) {
                        leads_list.append('<li class="list-group-item lead-list-error-box">No Leads Found</li>');
                    }

                    var lead_clone = $(ui.helper);
                    lead_clone.html('<h4>' + lead_clone.find("h4").text() + '</h4>');
                    lead_clone.addClass("dragging-lead");
                },
                stop: function (event, ui) {
                    $(this).show();

                    var leads_list = $('#leads-list');
                    leads_list.find('.lead-list-error-box').remove();
                }
            });
        }

        function updatePriorityText(rep_id, priority) {
            if (priority === '') {
                var text = $('#unassigned-' + rep_id).text();
                text = text.split(' ');
                var new_priority = parseInt(text[1]) + 1;
                $('#unassigned-' + rep_id).text("Unassigned: " + new_priority);
            }

            else if (priority === 'low') {
                var text = $('#low-' + rep_id).text();
                text = text.split(' ');
                var new_priority = parseInt(text[1]) + 1;
                $('#low-' + rep_id).text("Low: " + new_priority);
            }

            else if (priority === 'medium') {
                var text = $('#med-' + rep_id).text();
                text = text.split(' ');
                var new_priority = parseInt(text[1]) + 1;
                $('#med-' + rep_id).text("Medium: " + new_priority);
            }

            else if (priority === 'high') {
                var text = $('#high-' + rep_id).text();
                text = text.split(' ');
                var new_priority = parseInt(text[1]) + 1;
                $('#high-' + rep_id).text("High: " + new_priority);
            }
        }

        $('#lead-create-modal').on('show.bs.modal', function (event) {
            var button = $(event.relatedTarget);
            var url = button.data('url');

            var modal = $(this);
            modal.find('.modal-content').load(url);
        });

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

            // Keep these comments for now

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

            modal.find('#delete-lead-confirmation-button').click(function () {
                window.location.replace(url);
            });
        });

        function loadLeadList(event) {
            var target = $(event.currentTarget);
            var lead_search = $('#lead-search');
            var url = lead_search.data('url');
            var group = target.data('group');
            var lead_search_box = lead_search.find('input');
            var value = '';
            var params = '';

            if (group != null) {
                value = target.data('value');
                params += group + '/' + value;
            }
            else {
                var search_term = lead_search_box.val();
                params += 'search';

                if (search_term != '') {
                    params += '/' + $.trim(search_term).replace(/ /g, "+");
                }
            }

            var lead_list = $('#leads-list');
            lead_search.find('img').show();
            lead_search.find('.glyphicon-search').hide();

            lead_list.load(url + params, function (response, status, xhr) {
                if (status == "success") {
                    loadLeadsListInteractions();

                    if (group != null) {
                        if (group != 'sort-order') {
                            $("ul li a[data-group=" + group + "] .lead-search-dropdown-check").remove();
                            $("ul li a[data-group=" + group + "][data-value=" + value + "]").prepend("<i class='lead-search-dropdown-check glyphicon glyphicon-ok'></i>");
                        }
                        else {
                            if (value == 'asc') {
                                $("[data-group='sort-order'][data-value=desc]").removeClass("active");
                                $("[data-group='sort-order'][data-value=asc]").addClass("active");
                            }
                            else {
                                $("[data-group='sort-order'][data-value=asc]").removeClass("active");
                                $("[data-group='sort-order'][data-value=desc]").addClass("active");
                            }
                        }

                        if (group == 'search-field') {
                            lead_search_box.attr('placeholder', "Search by " + value);
                        }
                    }

                    $(this).find('[data-tooltip="tooltip"]').tooltip({container: 'body'});
                }
                else if (status == "error") {
                    $('#leads-list').html("<div class='lead-list-message-div'>Sorry but there was an error: " + xhr.status + " " + xhr.statusText + "</div>");
                }

                lead_search.find('img').hide();
                lead_search.find('.glyphicon-search').show();
            });
        }

        var timeout_thread = null;
        var previous_val = '';
        $('#lead-search input').on('keyup', function (event) {
            if (event.keyCode == '13') {
                loadLeadList(event);
                return;
            }

            if ($.trim($(this).val()) == '' && $.trim(previous_val) == '') {
                previous_val = $(this).val();
                return;
            }
            previous_val = $(this).val();

            clearTimeout(timeout_thread);
            timeout_thread = setTimeout(function () {
                loadLeadList(event);
            }, 100);
        });

        $('.lead-filter, #lead-search-button').click(function(event) {
            loadLeadList(event);
            event.stopPropagation();
        });

        $('#commit-leads-button').click(function () {
            commitAssignedLeads();
        });

        // Commits all the currently assigned leads in the database to hubspot.
        function commitAssignedLeads() {
            var commit_leads_button = $('#commit-leads-button');
            var url_base_path = commit_leads_button.data('url-base-path');
            var module_base_path = commit_leads_button.data('module-base-path');
            var imgTag = '<img id="commit-loading-gif" alt="loading" src="' + url_base_path + module_base_path + '/templates/assets/images/ajax-loader.gif">';
            $(imgTag).insertBefore(commit_leads_button);
            commit_leads_button.hide();
            $.ajax({
                url: url_base_path + "acrocrm_hubspot_integration/commit_assigned_leads",
                dataType: 'text',
                success: function (data) {
                    var returnObj = $.parseJSON(data.trim());
                    if (returnObj.status === 'success') {
                        $(".sales-rep-lead-list").each(function () {
                            $(this).find('[data-lead-id]').each(function () {
                                $(this).remove();
                            });
                            $(this).not(":has(.no-assigned-leads)").append('<li class="list-group-item no-assigned-leads">No Assigned Leads</li>');
                        });
                        $(".priority-indicator").text('0');
                        displayAlertMsg('success', 'Leads created on HubSpot successfully.');
                    } else if (returnObj.status === 'no_leads_to_commit') {
                        displayAlertMsg('info', 'There are no leads to send to HubSpot.');
                    } else {
                        if (returnObj.leads) {
                            for (var i = 0; i < returnObj.leads.length; i++) {
                                $(".sales-rep-lead-list").find('[data-lead-id="' + returnObj.leads[i].id + '"]').remove();
                                var prioritySpan = $('#' + returnObj.leads[i].priority + '-' + returnObj.leads[i].uid);
                                var count = prioritySpan.text();
                                prioritySpan.text(count - 1);
                            }
                            $(".sales-rep-lead-list").not(":has([data-lead-id])").append('<li class="list-group-item no-assigned-leads">No Assigned Leads</li>');
                        }
                        displayAlertMsg('error', returnObj.message);
                    }
                    $('#commit-loading-gif').remove();
                    commit_leads_button.show();
                },
                error: function (jqXHR, textStatus, errorThrown) {
                    displayAlertMsg('error', "Something went wrong when trying to send the leads to HubSpot.");
                    $('#commit-loading-gif').remove();
                    commit_leads_button.show();
                }
            });
        }

        // display message at the top of the content window
        // valid message types are: 'error' (red), 'success' (green), 'warning' (yellow)
        // msg type defaults to 'info' (blue) if none of these cases are matched
        function displayAlertMsg(msgType, msg) {
            $("#alert-msg-div").empty();
            var html = '<div id="alert-msg-div" class="col-md-12">';
            if (msgType == 'error') {
                html += '<div class="alert alert-danger">';
            }
            else if (msgType == 'success') {
                html += '<div class="alert alert-success">';
            }
            else if (msgType == 'warning') {
                html += '<div class="alert alert-warning">';
            }
            else {
                html += '<div class="alert alert-info">';
            }
            html += msg;
            html += '</div></div>';
            $('#page-wrapper').prepend(html);
        }
    }
};

// @TODO: Run text through Drupal.t().
function createHubspotContact(lead_id, url_base_path) {
    var imgTag = '<img id="loading-gif-' + lead_id + '" alt="loading" src="' + url_base_path + 'acrocrm/templates/assets/images/ajax-loader.gif">';
    $(imgTag).insertBefore($("#" + lead_id).parent());
    $("#" + lead_id).hide();
    $.ajax({
        url: url_base_path + "acrocrm_hubspot_integration/create_contact/" + lead_id,
        success: function (data) {
            $("#loading-gif-" + lead_id).remove();
            $("#" + lead_id).show();
            var message = data.trim();
            $('#message-container').remove();
            if (message == "success") {
                var prefix = '<div id="message-container" class="row"><div class="col-lg-10 col-md-12"><div class="alert alert-success">';
                var suffix = '<br></div></div></div>';
                $(prefix + 'The HubSpot contact was created successfully' + suffix).insertAfter('#header-row');
                $('#lead_' + lead_id + '_container').remove();

                if ($('.lead-container').length == 0) {
                    $('<div class="no-leads">There are no leads to display.</div>').insertAfter('#delete-lead-confirmation-modal');
                }
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

// update a lead's priority
// NOTE: must be outside of Drupal.behaviors.acrocrm_leads
function assignPriority(lead_id, element) {
    $.ajax({
        url: "/acrocrm_leads/assign_lead_priority/" + lead_id + "/" + $(element).val()
    });
}
