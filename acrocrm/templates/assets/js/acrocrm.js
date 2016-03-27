// --- Acrocrm Leads Module --- //
Drupal.behaviors.acrocrm_leads = {
  'attach': function (context) {
    $(document).ready(function (event) {
      // initiate tool tips
      $('[data-tooltip="tooltip"]').tooltip({container: 'body'});

      // create sales rep accordion
      $('.sales-rep').accordion({
        header: '> h4',
        collapsible: true,
        active: false,
        heightStyle: 'content',
        cursor: 'move',
        beforeActivate: function (event, ui) {
          ui.newHeader.addClass('sales-rep-open').removeClass('sales-rep-closed');
          ui.oldHeader.addClass('sales-rep-closed').removeClass('sales-rep-open');
        },
        activate: function(event, ui) {
          // determine how many accordions are open, if they all are open change "Expand All" to "Collapse All"
          // if none are open change "Collapse All" to "Expand All"
          var numOpen = 0;
          var sales_rep = $('.sales-rep');
          var expand_all = $('#expand-all');
          sales_rep.each(function () {
            if ($(this).accordion( 'option', 'active' ) !== false) {
              numOpen += 1;
            }
          });
          if (numOpen == sales_rep.length) {
            expand_all.removeClass('expand-repdiv');
            expand_all.addClass('collapse-repdiv');
            expand_all.html('Collapse All');
          }
          else if (numOpen == 0) {
            expand_all.removeClass('collapse-repdiv');
            expand_all.addClass('expand-repdiv');
            expand_all.html('Expand All');
          }
        }
      }).droppable({
        accept: '.unassigned-lead, .uncommitted-lead',
        refreshPositions: true,
        hoverClass: 'ui-state-highlight',
        tolerance: 'pointer',
        activate: function (event, ui) {
          $('.ui-accordion-header, .btn').css('cursor', 'move');
          $('.accordion').accordion('option', 'cursor', 'move');
          $('[data-tooltip="tooltip"]').tooltip('disable');
        },
        deactivate: function (event, ui) {
          $('.ui-accordion-header, .btn').css('cursor', 'pointer');
          $('.accordion').accordion('option', 'cursor', 'pointer');
          $('[data-tooltip="tooltip"]').tooltip('enable');
        },
        drop: function (event, ui) {
          var sales_rep = $(this);
          var lead_draggable = $(ui.draggable);

          sales_rep.find('> h4').removeClass('sales-rep-hover').bind('mouseenter mouseleave');

          var original_rep_id = (lead_draggable.data('assigned-rep-id') != undefined) ? lead_draggable.data('assigned-rep-id') : null;
          if (original_rep_id != null && original_rep_id == sales_rep.data('rep-id')) {
            return;
          }

          var url = Drupal.settings.basePath + 'acrocrm_leads/assign_lead/';
          var params = sales_rep.data('rep-id') + '/' + lead_draggable.data('lead-id');

          sales_rep.find('> h4 img').show();

          sales_rep.find('.sales-rep-lead-list').load(url + params, function (response, status, xhr) {
            if (status == 'success') {
              if (original_rep_id == null) {
                if ($('.lead-filter[data-group="show"][data-value="all"] .glyphicon-ok').length != 0) {
                  loadLeadList(event);
                }
                else {
                  lead_draggable.remove();
                }

                var leads_list = $('#leads-list');
                if (leads_list.find('li').length == 0) {
                  leads_list.html('<li class="list-group-item lead-list-error-box">' + Drupal.t('No Leads Found') + '</li>');
                }
              }
              else {
                lead_draggable.remove();

                var original_sales_rep = $('.sales-rep[data-rep-id="' + original_rep_id + '"]');
                if (original_sales_rep.find('li').length == 0) {
                  original_sales_rep.find('ul').html('<li class="list-group-item no-assigned-leads">' + Drupal.t('No Assigned Leads') + '</li>');
                }
              }

              loadSalesRepInteractions(sales_rep);

              // update priority counts for the rep that the lead was assigned to and if applicable the rep
              // that the lead was taken from
              var url = Drupal.settings.basePath + 'acrocrm_leads/get_lead_priority/';
              var params = lead_draggable.data('lead-id');
              $.get(url + params, function (response, status, xhr) {
                var message = response.trim();

                if (message === 'low' || message === 'medium' || message === 'high' || message === 'unassigned') {
                  var sales_rep_priority_indicator = $('#' + message + '-' + sales_rep.data('rep-id'));
                  sales_rep_priority_indicator.html(parseInt(sales_rep_priority_indicator.html()) + 1);

                  if (original_rep_id != null) {
                    var original_rep_priority_indicator = $('#' + message + '-' + original_rep_id);
                    original_rep_priority_indicator.html(parseInt(original_rep_priority_indicator.html()) - 1);
                  }
                }
                else {
                  console.log('An error occurred when trying to get lead priority');
                  displayAlertMsg('error', Drupal.t('An error occurred when trying to get lead priority, unable ' +
                    'to update lead priority.'))
                }
              }, 'text');
            }
            else if (status == 'error') {
              event.preventDefault();
              displayAlertMsg('error', Drupal.t('Unable to assign lead.'));

              sales_rep.data('accordion-hovering', 'false');
              if (sales_rep.data('accordion-prev-active') === 'false') {
                sales_rep.accordion('option', 'active', false); // Close accordion
              }
            }

            sales_rep.find('> h4 img').hide();
          });
        },
        over: function (event, ui) {
          var sales_rep = $(this);
          sales_rep.find('> h4').addClass('sales-rep-hover').unbind('mouseenter mouseleave');

          if (sales_rep.accordion('option', 'active') === 0) {
            sales_rep.data('accordion-prev-active', 'true'); // Accordion was previously active
          }
          else {
            sales_rep.data('accordion-hovering', 'true');

            setTimeout(function () {
              if (sales_rep.data('accordion-hovering') === 'true') {
                sales_rep.accordion('option', 'active', 0); // Expand accordion
                sales_rep.data('accordion-prev-active', 'false'); // Accordion was not previously active
                sales_rep.data('accordion-hovering', 'false');

                if (sales_rep.find($('.fa.fa-angle-down')).length > 0) {
                  sales_rep.find($('.fa')).removeClass('fa-angle-down').addClass('fa-angle-right');
                }
                else {
                  sales_rep.find($('.fa')).removeClass('fa-angle-right').addClass('fa-angle-down');
                }
              }
            }, 750);
          }
        },
        out: function (event, ui) {
          var sales_rep = $(this);
          sales_rep.find('> h4').removeClass('sales-rep-hover').bind('mouseenter mouseleave');

          sales_rep.data('accordion-hovering', 'false');
          if (sales_rep.data('accordion-prev-active') === 'false') {
            sales_rep.accordion('option', 'active', false); // Close accordion
          }
        }
      });

      $('#leads-list').droppable({
        accept: '.uncommitted-lead',
        refreshPositions: true,
        hoverClass: 'ui-state-highlight',
        tolerance: 'pointer',
        drop: function (event, ui) {
          var leads_list = $(this);
          var lead_draggable = $(ui.draggable);

          lead_draggable.hide();
          var url = Drupal.settings.basePath + 'acrocrm_leads/unassign_lead/';
          var params = lead_draggable.data('lead-id');

          leads_list.load(url + params, function (response, status, xhr) {
            if (status == 'success') {
              lead_draggable.remove();
              loadLeadList(event);
              var original_rep_id = (lead_draggable.data('assigned-rep-id') != undefined) ? lead_draggable.data('assigned-rep-id') : null;
              var original_sales_rep = $('.sales-rep[data-rep-id="' + original_rep_id + '"]');

              if (original_sales_rep.find('li').length == 0) {
                original_sales_rep.find('ul').html('<li class="list-group-item no-assigned-leads">' + Drupal.t('No Assigned Leads') + '</li>');
              }

              // update priority counts for the rep that the lead was assigned to and if applicable the rep
              // that the lead was taken from
              var url = Drupal.settings.basePath + 'acrocrm_leads/get_lead_priority/';
              var params = lead_draggable.data('lead-id');
              $.get(url + params,
                function (response, status, xhr) {
                  var message = response.trim();

                  if (message === 'low' || message === 'medium' || message === 'high' || message === 'unassigned') {
                    var priority_indicator = $('#' + message + '-' + original_rep_id);
                    priority_indicator.html(parseInt(priority_indicator.html()) - 1);
                  }
                  else {
                    console.log('An error occurred when trying to get lead priority');
                    displayAlertMsg('error', Drupal.t('An error occurred when trying to get lead priority, unable ' +
                      'to update lead priority.'))
                  }
                },
                'text');
            }
            else if (status == 'error') {
              event.preventDefault();
              lead_draggable.show();
              displayAlertMsg('error', Drupal.t('Unable to assign lead.'));
            }
          });
        }
      });

      $('.list-group-item-heading.ui-accordion-header').on('click', function () {
        if ($(this).find($('.fa.fa-angle-down')).length > 0) {
          $(this).find($('.fa')).removeClass('fa-angle-down').addClass('fa-angle-right');
        }
        else {
          $(this).find($('.fa')).removeClass('fa-angle-right').addClass('fa-angle-down');
        }
      });

      $('#expand-all').on('click', function () {
        if ($('#expand-all').hasClass('expand-repdiv')) {
          $('.sales-rep').each(function () {
            $(this).accordion({active: 0});
          });
          $(this).removeClass('expand-repdiv');
          $(this).addClass('collapse-repdiv');
          $(this).html('Collapse All');

          // make the div arrow icon point down
          $('.list-group-item-heading.ui-accordion-header').find($('.fa')).removeClass('fa-angle-right').addClass('fa-angle-down');
        }
        else {
          $('.sales-rep').each(function () {
            $(this).accordion({active: 1});
          });
          $(this).removeClass('collapse-repdiv');
          $(this).addClass('expand-repdiv');
          $(this).html('Expand All');
          // make the div arrow icon point right
          $('.list-group-item-heading.ui-accordion-header').find($('.fa')).removeClass('fa-angle-down').addClass('fa-angle-right');
        }
      });

      loadLeadList(event);
      loadSalesRepInteractions();
    }); // end document.ready

    function loadSalesRepInteractions(sales_reps) {
      sales_reps = typeof sales_reps !== 'undefined' ? sales_reps : $('.sales-rep-list');

      sales_reps.find('.readmore').readmore({
        collapsedHeight: 18
      });

      sales_reps.find('.sales-rep-lead-list img').hide();

      sales_reps.find('[data-tooltip="tooltip"]').tooltip({container: 'body'});

      sales_reps.find('.sales-rep-lead').accordion({
        header: '> h5',
        collapsible: true,
        active: false,
        heightStyle: 'content',
        cursor: 'move',
        activate: function (event, ui) {
          $('.readmore').readmore({
            collapsedHeight: 18
          });
        }
      });

      // Uncommitted leads are the leads that have been assigned but are not sent to hubspot, etc.
      sales_reps.find('.uncommitted-lead').draggable({
        appendTo: 'body',
        zIndex: 100,
        helper: 'clone',
        revert: 'invalid',
        cursor: 'move',
        cursorAt: {top: 0, left: 0},
        refreshPositions: true,
        start: function (event, ui) {
          $(this).hide();

          var original_rep_id = ($(this).data('assigned-rep-id') != undefined) ? $(this).data('assigned-rep-id') : null;
          var original_sales_rep = $('.sales-rep[data-rep-id="' + original_rep_id + '"]');
          if (original_sales_rep.find('li').length <= 1) {
            original_sales_rep.find('ul').append('<li class="list-group-item no-assigned-leads">' + Drupal.t('No Assigned Leads') + '</li>');
          }

          var lead_clone = $(ui.helper);
          lead_clone.html('<h4>' + lead_clone.find('h5').text() + '</h4>');
          lead_clone.addClass('dragging-lead');
        },
        stop: function (event, ui) {
          $(this).show();

          var original_rep_id = ($(this).data('assigned-rep-id') != undefined) ? $(this).data('assigned-rep-id') : null;
          var original_sales_rep = $('.sales-rep[data-rep-id="' + original_rep_id + '"]');
          original_sales_rep.find('.no-assigned-leads').remove();
        }
      });

      sales_reps.find('.priority-radio-btn').mouseup(function() {
        old_priority = $(this).find('input[type=radio]:checked').attr('value');
      }).change(function (event) {
        updatePriorityRadio(event, this);
      });
    }

    // loads all interactions for leads on leads overview page
    function loadLeadsListInteractions() {
      $('#leads-list .readmore').readmore({
        collapsedHeight: 18
      });

      $('.unassigned-lead').draggable({
        appendTo: 'body',
        zIndex: 100,
        helper: 'clone',
        revert: 'invalid',
        cursor: 'move',
        cursorAt: {top: 0, left: 0},
        refreshPositions: true,
        start: function (event, ui) {
          var leads_list = $('#leads-list');
          var has_no_leads = false;

          if ($('.lead-filter[data-group=show][data-value=all] .glyphicon-ok').length === 0) {
            $(this).hide();

            if (leads_list.find('li').length <= 1) {
              has_no_leads = true;
            }
          }
          else {
            if (leads_list.find('li').length === 0) {
              has_no_leads = true;
            }
          }

          if (has_no_leads) {
            leads_list.append('<li class="list-group-item lead-list-error-box">' + Drupal.t('No Leads Found') + '</li>');
          }

          var lead_clone = $(ui.helper);
          lead_clone.html('<h4>' + lead_clone.find('h4').text() + '</h4>');
          lead_clone.addClass('dragging-lead');
        },
        stop: function (event, ui) {
          $(this).show();

          var leads_list = $('#leads-list');
          leads_list.find('.lead-list-error-box').remove();
        }
      });

      $('.recover-lead-button').click(function (event) {
        var url = Drupal.settings.basePath + 'acrocrm_leads/recover_lead/';
        var params = $(this).data('lead-id');

        $.ajax({
          type: 'GET',
          url: url + params,
          success: function (data, textStatus) {
            window.location.replace(url + params);
          },
          error: function (xhr, textStatus, errorThrown) {
            displayAlertMsg('error', Drupal.t('Error recovering lead.'));
          }
        });
      });

      $('#leads-list .priority-radio-btn').mouseup(function() {
        old_priority = $(this).find('input[type=radio]:checked').attr('value');
      }).change(function (event) {
        return updatePriorityRadio(event, this);
      });
    }

    var old_priority = null;

    function updatePriorityRadio(event, priority_radio_buttons) {
      var radio_button = $(priority_radio_buttons).find('input[type=radio]:checked');
      var name = radio_button.attr('name').split('-');
      var is_assigned = (name[0] === 'assigned');
      var lead_id = name[1];
      var priority = radio_button.attr('value');

      $.ajax({
        url: Drupal.settings.basePath + "acrocrm_leads/set_lead_priority/" + priority + "/" + lead_id,
        success: function () {
          if (is_assigned) {
            $('.priority-radio-btn input[type=radio][name="unassigned-' + lead_id + '"][value="' + priority + '"]').prop("checked", true);
          }
          else {
            $('.priority-radio-btn input[type=radio][name="assigned-' + lead_id + '"][value="' + priority + '"]').prop("checked", true);
          }

          var sales_rep_id = $('.sales-rep-lead[data-lead-id=' + lead_id + ']').data('assigned-rep-id');

          var priority_indicator = $('#' + priority + '-' + sales_rep_id);
          priority_indicator.html(parseInt(priority_indicator.html()) + 1);

          var old_priority_indicator = $('#' + old_priority + '-' + sales_rep_id);
          old_priority_indicator.html(parseInt(old_priority_indicator.html()) - 1);

          return true;
        },
        error: function () {
          return false;
        }
      });
    }

    $('#lead-create-modal').on('show.bs.modal', function (event) {
      var button = $(event.relatedTarget);
      var url = Drupal.settings.basePath + 'acrocrm_leads/new_lead/';
      var params = button.data('lead-id');

      var modal = $(this);
      modal.find('.modal-content').load(url + params);
    });

    $('#lead-edit-modal').on('show.bs.modal', function (event) {
      var button = $(event.relatedTarget);
      var url = Drupal.settings.basePath + 'acrocrm_leads/edit_lead/';
      var params = button.data('lead-id');

      var modal = $(this);
      modal.find('.modal-content').load(url + params);
    });

    $('#delete-lead-confirmation-modal').on('show.bs.modal', function (event) {
      var trigger_button = $(event.relatedTarget);
      var url = Drupal.settings.basePath + 'acrocrm_leads/delete_lead/';
      var params = trigger_button.data('lead-id');

      var modal = $(this);
      modal.find('#delete-lead-confirmation-button').click(function () {
        window.location.replace(url + params);
      });
    });

    // Keeps track of the start of the range for the leads query
    var leads_list_index = 0;
    var num_of_records_to_show = 4;

    function insertPagination(list) {
      $('#leads-list-pager').remove();
      var leads_pager_string = '<nav id="leads-list-pager"><ul class="pager">';

      if (leads_list_index != 0) {
        leads_pager_string += '<li><a id="pg-previous" href="#">' + Drupal.t('Previous') + '</a></li>';
      }

      if (list.find('> li').length > num_of_records_to_show) {
        leads_pager_string += '<li><a id="pg-next" href="#">' + Drupal.t('Next') + '</a></li>';
        list.find('> li:last-child:not(.lead-list-error-box)').remove();
      }

      leads_pager_string += '</ul></nav>';
      list.after(leads_pager_string);

      $('#pg-previous').click(function (event) {
        leads_list_index -= num_of_records_to_show;
        loadLeadList(event);
        event.preventDefault();
      });

      $('#pg-next').click(function (event) {
        leads_list_index += num_of_records_to_show;
        loadLeadList(event);
        event.preventDefault();
      });
    }

    function loadLeadList(event) {
      var target = $(event.currentTarget);
      var lead_search = $('#lead-search');
      var url = Drupal.settings.basePath + 'acrocrm_leads/filter_leads/';
      var group = target.data('group');
      var lead_search_box = lead_search.find('input');
      var value = '';
      var params = '';
      var lower_index_of_range = leads_list_index;
      var upper_index_of_range = lower_index_of_range + num_of_records_to_show + 1; // We load an extra record so that we can check if we are at the end of the query results.

      params += lower_index_of_range + '/' + upper_index_of_range + '/';

      if (group != null) {
        value = target.data('value');
        params += group + '/' + value;
      }
      else {
        var search_term = lead_search_box.val();
        params += 'search';

        if (search_term != '') {
          params += '/' + $.trim(search_term).replace(/ /g, '+');
        }
      }

      var lead_list = $('#leads-list');
      lead_list.hide();
      lead_list.prev('.leads-list-spinner').show();

      lead_list.load(url + params, function (response, status, xhr) {
        lead_list.prev('.leads-list-spinner').hide();
        lead_list.show();

        if (status == 'success') {
          insertPagination(lead_list);
          loadLeadsListInteractions();

          if (group != null) {
            if (group != 'sort-order') {
              $('ul li a[data-group="' + group + '"] .lead-search-dropdown-check').remove();
              $('ul li a[data-group="' + group + '"][data-value="' + value + '"]').prepend('<i class="lead-search-dropdown-check glyphicon glyphicon-ok"></i>');
            }
            else {
              if (value == 'asc') {
                $('[data-group="sort-order"][data-value=desc]').removeClass('active');
                $('[data-group="sort-order"][data-value=asc]').addClass('active');
              }
              else {
                $('[data-group="sort-order"][data-value=asc]').removeClass('active');
                $('[data-group="sort-order"][data-value=desc]').addClass('active');
              }
            }

            if (group == 'search-field') {
              lead_search_box.attr('placeholder', 'Search by ' + value);
            }
          }

          $(this).find('[data-tooltip="tooltip"]').tooltip({container: 'body'});
        }
        else if (status == 'error') {
          $('#leads-list').html('<div class="lead-list-message-div">' + Drupal.t('Sorry, but there was an error: ' + xhr.status + ' ' + xhr.statusText) + '</div>');
        }
      });
    }

    var timeout_thread = null;
    var previous_val = '';
    $('#lead-search input').on('keyup', function (event) {
      leads_list_index = 0;

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

    $('.lead-filter, #lead-search-button').click(function (event) {
      leads_list_index = 0;
      loadLeadList(event);
      event.stopPropagation();
    });

    $('#commit-leads-button').click(function () {
      commitAssignedLeads();
    });

    // Commits all the currently assigned leads in the database to hubspot.
    function commitAssignedLeads() {
      var commit_leads_button = $('#commit-leads-button');
      var module_base_path = commit_leads_button.data('module-base-path');
      var imgTag = '<img id="commit-loading-gif" alt="loading" src="' + Drupal.settings.basePath + module_base_path + '/templates/assets/images/ajax-loader.gif">';
      $(imgTag).insertBefore(commit_leads_button);
      commit_leads_button.hide();
      $.ajax({
        url: Drupal.settings.basePath + 'acrocrm_hubspot_integration/commit_assigned_leads',
        dataType: 'text',
        success: function (data) {
          var returnObj = $.parseJSON(data.trim());
          if (returnObj.status === 'success') {
            $('.sales-rep-lead-list').each(function () {
              $(this).find('[data-lead-id]').each(function () {
                $(this).remove();
              });
              $(this).not(':has(.no-assigned-leads)').append('<li class="list-group-item no-assigned-leads">' + Drupal.t('No Assigned Leads') + '</li>');
            });
            $('.priority-indicator').text('0');
            displayAlertMsg('success', Drupal.t('Leads created on HubSpot successfully.'));
          } else if (returnObj.status === 'no_leads_to_commit') {
            displayAlertMsg('info', Drupal.t('There are no leads to send to HubSpot.'));
          } else if (returnObj.status === 'get_oauth_token') {
            window.location.replace(returnObj.url);
          } else {
            if (returnObj.leads) {
              for (var i = 0; i < returnObj.leads.length; i++) {
                $('.sales-rep-lead-list').find('[data-lead-id="' + returnObj.leads[i].id + '"]').remove();
                var prioritySpan = $('#' + returnObj.leads[i].priority + '-' + returnObj.leads[i].uid);
                var count = prioritySpan.text();
                prioritySpan.text(count - 1);
              }
              $('.sales-rep-lead-list').not(':has([data-lead-id])').append('<li class="list-group-item no-assigned-leads">' + Drupal.t('No Assigned Leads') + '</li>');
            }
            displayAlertMsg('error', Drupal.t(returnObj.message));
          }
          $('#commit-loading-gif').remove();
          commit_leads_button.show();
        },
        error: function (jqXHR, textStatus, errorThrown) {
          displayAlertMsg('error', Drupal.t('Something went wrong when trying to send the leads to HubSpot.'));
          $('#commit-loading-gif').remove();
          commit_leads_button.show();
        }
      });
    }

    // display message at the top of the content window
    // valid message types are: 'error' (red), 'success' (green), 'warning' (yellow)
    // msg type defaults to 'info' (blue) if none of these cases are matched
    function displayAlertMsg(msgType, msg) {
      $('#alert-msg-div').empty();
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
      html += '<a href="#" class="close" data-dismiss="alert" aria-label="close">&times;</a>';
      html += msg;
      html += '</div></div>';
      $('#page-wrapper').prepend(html);
    }
  }
}; // end Drupal.behaviors.acrocrm_leads

// send the a lead to hubspot
// url base path changes depending on whether or not the user has
// clean urls turned on
function createHubspotContact(lead_id, url_base_path) {
  var imgTag = '<img id="loading-gif-' + lead_id + '" alt="loading" src="' + url_base_path + 'acrocrm/templates/assets/images/ajax-loader.gif">';
  $(imgTag).insertBefore($('#' + lead_id).parent());
  $('#' + lead_id).hide();
  $.ajax({
    url: url_base_path + 'acrocrm_hubspot_integration/create_contact/' + lead_id,
    success: function (data) {
      $('#loading-gif-' + lead_id).remove();
      $('#' + lead_id).show();
      var message = data.trim();
      $('#message-container').remove();
      if (message == 'success') {
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

        if (message == 'contact_already_exists') {
          $(prefix + Drupal.t('The contact you are trying to create on HubSpot already exists. ' +
            'This could be due to a duplicate email address.') + suffix).insertAfter('#header-row');
        } else if (message == 'email_invalid') {
          $(prefix + Drupal.t('The email address of the contact you are trying to create on HubSpot is invalid. ' +
            'HubSpot has stricter email validation than AcroCRM.') + suffix).insertAfter('#header-row');
        } else if (message == 'email_invalid') {
          $(prefix + Drupal.t('The the contact you are trying to create could not be found. ' +
            'Refresh the page and try again') + suffix).insertAfter('#header-row');
        } else {
          $(prefix + message + suffix).insertAfter('#header-row');
        }
      }
    },
    dataType: 'text'
  });
}
