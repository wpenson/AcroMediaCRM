<?php

// @TODO: Document
function acrocrm_leads_overview_page() {
  // load the leads from the database
  $leads = acrocrm_leads_load_leads();

  $page['content']['leads'] = array(
    '#markup' => acrocrm_leads_generate_leads_html($leads)
  );

  $page['content']['reps'] = array(
    '#markup' => acrocrm_leads_get_sales_reps()
  );

  return theme('acrocrm_leads_home', array('page' => $page));
}

// @TODO: Document
function acrocrm_leads_assign_lead_ajax($rep_id, $lead_id) {
  // check to see if the lead is already assigned to someone
  $check_exists = db_query('SELECT * FROM assigned_leads WHERE lead_id=:id', array(':id' => $lead_id));

  // if lead exists just update assignment
  if ($check_exists->rowCount() > 0) {
    $update_assignment = db_update('assigned_leads')
      ->fields(array(
        'uid' => $rep_id
      ))
      ->condition('lead_id', $lead_id, '=')
      ->execute();
  }
  // else insert the assigned lead and assign to correct sales rep
  else {
    $assign_lead = db_insert('assigned_leads')
      ->fields(array(
        'uid' => $rep_id,
        'lead_id' => $lead_id
      ))
      ->execute();
  }

  // regenerate leads list and print it out
  $leads_list = '';
  acrocrm_leads_generate_sales_rep_lead_list($rep_id, $leads_list);
  print $leads_list;
}

function acrocrm_leads_unassign_lead_ajax($lead_id) {
  // Delete the lead assignments from the assignment table
  db_delete('assigned_leads')
    ->condition('lead_id', $lead_id)
    ->execute();
}

/**
 * Get the priority of a given lead
 */
function acrocrm_leads_get_lead_priority($lead_id) {
  $check_exists = db_query('SELECT * FROM leads WHERE lead_id=:id', array(':id' => $lead_id));

  if ($check_exists->rowCount() > 0) {
    $get_priority = db_query('SELECT priority FROM leads WHERE lead_id=:id', array(':id' => $lead_id));

    $priority = $get_priority->fetchAssoc();
    if (strlen($priority["priority"]) == 0) {
      print "unassigned";
    }
    else {
      print $priority['priority'];
    }
  }
  else {
    print "error"; // error case
  }
}

function acrocrm_leads_priority_get_ajax($priority, $lead_id) {
  $check_exists = db_query('SELECT * FROM leads WHERE lead_id=:id', array(':id' => $lead_id));

  if ($priority === 'unassigned') {
    $priority = '';
  }

  if ($check_exists->rowCount() > 0) {
    db_update('leads')
      ->fields(array(
        'priority' => $priority
      ))
      ->condition('lead_id', $lead_id, '=')
      ->execute();
  }
}

/*
 * Get a list of all leads from the database
 */
function acrocrm_leads_load_leads() {
  $assigned_leads = db_select('assigned_leads', 'lead_id')
    ->fields('lead_id')
    ->execute();


  if ($assigned_leads->rowCount() > 0) {
    $lead_ids = array();

    while ($lead_id = $assigned_leads->fetchAssoc()) {
      //$lead_ids .= $lead_id["lead_id"] . ', ';
      array_push($lead_ids, $lead_id["lead_id"]);
    }

    $leads = db_select('leads', 'l')
      ->fields('l')
      ->condition('lead_id', $lead_ids, 'NOT IN')
      ->execute();
  }

  else {
    $leads = db_select('leads', 'l')
      ->fields('l')
      ->execute();
  }

  $leads_arr = array();

  if ($leads) {
    while ($lead = $leads->fetchAssoc()) {

      $leads_arr[] = array(
        'lead_id' => $lead['lead_id'],
        'firstname' => $lead['firstname'],
        'name' => $lead['name'],
        'email' => $lead['email'],
        'phone' => $lead['phone'],
        'comments' => $lead['comments'],
        'priority' => $lead['priority']
      );
    }
  }
  else {
    $leads_arr[] = 'An error occurred, could not retrieve leads from database.';
  }
  return $leads_arr;
}

/**
 * Gets recently created leads from the database.
 * @param $leads
 * @return string
 */
function acrocrm_leads_recently_created_leads($limit) {
  $leads = db_query('SELECT firstname, leads.name AS leadname, email, phone, comments, priority, created_time, users.name AS username
                    FROM leads
                    LEFT JOIN users
                    ON created_by = uid
                    ORDER BY created_time DESC
                    LIMIT ' . $limit);

  $leads_arr = array();

  if ($leads) {
    foreach ($leads as $lead) {
      $leads_arr[] = array(
        'firstname' => $lead->firstname,
        'name' => $lead->leadname,
        'email' => $lead->email,
        'phone' => $lead->phone,
        'comments' => $lead->comments,
        'priority' => $lead->priority,
        'created_by' => $lead->username == NULL || $lead->username == '' ? 'Web Contact Form' : $lead->username,
        'created_time' => $lead->created_time,
      );
    }
  }
  else {
    $leads_arr[] = 'An error occurred, could not retrieve leads from database.';
  }

  return $leads_arr;
}

/**
 * Gets leads created by the given user.
 */
function acrocrm_leads_modified_by_user($uid, $limit) {
  $leads = db_query('SELECT firstname, name, email, phone, comments, priority, created_time
                    FROM leads
                    WHERE modified_by = ' . $uid . '
                    ORDER BY modified_time DESC
                    LIMIT ' . $limit);

  $leads_arr = array();

  if ($leads) {
    foreach ($leads as $lead) {
      $leads_arr[] = array(
        'firstname' => $lead->firstname,
        'name' => $lead->name,
        'email' => $lead->email,
        'phone' => $lead->phone,
        'comments' => $lead->comments,
        'priority' => $lead->priority,
        'modified_time' => $lead->modified_time,
      );
    }
  }
  else {
    $leads_arr[] = 'An error occurred, could not retrieve leads from database.';
  }

  return $leads_arr;
}

/**
 * Gets leads that are assigned to a given user.
 */
function acrocrm_leads_assigned_leads($uid) {
  $leads = db_query("SELECT leads.lead_id, firstname, leads.name AS leadname, email, phone, comments, priority, created_time, leads.status
                    FROM leads
                    INNER JOIN assigned_leads
                    ON assigned_leads.lead_id = leads.lead_id
                    INNER JOIN users
                    ON assigned_leads.uid = users.uid
                    WHERE users.uid = :uid
                    AND leads.status != 'CONVERTED'
                    AND leads.status != 'DELETED'",
    array(":uid" => $uid));

  $leads_arr = array();

  if ($leads) {
    foreach ($leads as $lead) {
      $leads_arr[] = array(
        'lead_id' => $lead->lead_id,
        'firstname' => $lead->firstname,
        'name' => $lead->leadname,
        'email' => $lead->email,
        'phone' => $lead->phone,
        'comments' => $lead->comments,
        'priority' => $lead->priority,
      );
    }
  }
  else {
    $leads_arr[] = 'An error occurred, could not retrieve leads from database.';
  }

  return $leads_arr;
}

/**
 * Gets recently modified leads from the database.
 * @param $leads
 * @return string
 */
function acrocrm_leads_recently_modified_leads($limit) {
  $leads = db_query('SELECT firstname, leads.name AS leadname, email, phone, comments, priority, modified_time, users.name AS username
                    FROM leads
                    LEFT JOIN users
                    ON modified_by = uid
                    ORDER BY modified_time DESC
                    LIMIT ' . $limit);

  $leads_arr = array();

  if ($leads) {
    foreach ($leads as $lead) {
      $leads_arr[] = array(
        'firstname' => $lead->firstname,
        'name' => $lead->leadname,
        'email' => $lead->email,
        'phone' => $lead->phone,
        'comments' => $lead->comments,
        'priority' => $lead->priority,
        'modified_by' => $lead->username == NULL || $lead->username == '' ? 'Web Contact Form' : $lead->username,
        'modified_time_time' => $lead->created_time,
      );
    }
  }
  else {
    $leads_arr[] = 'An error occurred, could not retrieve leads from database.';
  }

  return $leads_arr;
}

function acrocrm_leads_filter_leads_ajax($group = NULL, $value = NULL) {
  // Allowed values used for validation
  $valid_search_field_values = ['name', 'email', 'comments', 'phone'];
  $valid_show_values = ['all', 'unassigned', 'assigned', 'deleted'];
  $valid_sort_values = [
    'created_time',
    'modified_time',
    'priority',
    'name',
    'email'
  ];

  drupal_session_start();

  if ($group != NULL) {
    if ($group == 'search-field' && in_array($value, $valid_search_field_values)) {
      $_SESSION['lead_search_pref'] = $value;
    }
    else {
      if ($group == 'show' && in_array($value, $valid_show_values)) {
        $_SESSION['lead_show_pref'] = $value;
      }
      else {
        if ($group == 'sort' && in_array($value, $valid_sort_values)) {
          $_SESSION['lead_sort_pref'] = $value;
        }
        else {
          if ($group == 'sort-order' && ($value == 'asc' || $value == 'desc')) {
            $_SESSION['lead_sort_order_pref'] = $value;
          }
          else {
            if ($group == 'search') {
              $_SESSION['lead_search_val'] = $value;
            }
          }
        }
      }
    }
  }

  // Set defaults
  if (!isset($_SESSION['lead_search_pref'])) {
    $_SESSION['lead_search_pref'] = 'name';
  }

  if (!isset($_SESSION['lead_show_pref'])) {
    $_SESSION['lead_show_pref'] = 'unassigned';
  }

  if (!isset($_SESSION['lead_sort_pref'])) {
    $_SESSION['lead_sort_pref'] = 'created_time';
  }

  if (!isset($_SESSION['lead_sort_order_pref'])) {
    $_SESSION['lead_sort_order_pref'] = 'asc';
  }

  if (!isset($_SESSION['lead_search_val'])) {
    $_SESSION['lead_search_val'] = '';
  }

  // Build query
  $leads_query = db_select('leads', 'l');
  $leads_query->fields('l');
  $assigned_fields = FALSE;

  if ($_SESSION['lead_show_pref'] == 'unassigned') {
    $leads_query->condition('l.status', 'DELETED', '<>');
    $leads_query->leftJoin('assigned_leads', 'al', 'l.lead_id = al.lead_id');
    $leads_query->isNull('al.lead_id');
  }
  else {
    if ($_SESSION['lead_show_pref'] == 'assigned') {
      $leads_query->innerJoin('assigned_leads', 'al', 'l.lead_id = al.lead_id');
      $leads_query->fields('al', array('uid'));
      $assigned_fields = TRUE;
    }
    else {
      if ($_SESSION['lead_show_pref'] == 'deleted') {
        $leads_query->condition('l.status', 'DELETED', '=');
      }
      else {
        $leads_query->leftJoin('assigned_leads', 'al', 'l.lead_id = al.lead_id');
        $leads_query->fields('al', array('uid'));
        $assigned_fields = TRUE;
      }
    }
  }

  if ($_SESSION['lead_search_val'] != '') {
    if ($_SESSION['lead_search_pref'] == 'created_time' || $_SESSION['lead_search_val'] == 'modified_time') {
      // TODO: Add filtering by dates
    }
    else {
      if ($_SESSION['lead_search_pref'] == 'name') {
        $name = explode(' ', $_SESSION['lead_search_val']);

        $or = db_or();
        $or->condition('l.name', '%' . $name[0] . '%', 'LIKE');
        $or->condition('l.firstname', '%' . $name[0] . '%', 'LIKE');
        $leads_query->condition($or);

        if (isset($name[1])) {
          $or = db_or();
          $or->condition('l.name', '%' . $name[1] . '%', 'LIKE');
          $or->condition('l.firstname', '%' . $name[1] . '%', 'LIKE');
          $leads_query->condition($or);
        }
      }
      else {
        $leads_query->condition('l.' . $_SESSION['lead_search_pref'], '%' . $_SESSION['lead_search_val'] . '%', 'LIKE');
      }
    }
  }

  if ($_SESSION['lead_sort_pref'] == 'priority') {
    $leads_query->addExpression("CASE WHEN l.priority = 'high' THEN 1
                                     WHEN l.priority = 'medium' THEN 2
                                     WHEN l.priority = 'low' THEN 3
                                     WHEN l.priority = 'unassigned' THEN 4
                                     ELSE l.priority END",
      'order_col');
    $leads_query->orderBy('order_col', strtoupper($_SESSION['lead_sort_order_pref']));
  }
  else {
    $leads_query->orderBy('l.' . $_SESSION['lead_sort_pref'], strtoupper($_SESSION['lead_sort_order_pref']));
  }

  $leads_query = $leads_query->execute();
  $leads = array();

  if ($leads_query->rowCount() > 0) {
    while ($lead = $leads_query->fetchAssoc()) {
      $arr_entry = array(
        'lead_id' => $lead['lead_id'],
        'firstname' => $lead['firstname'],
        'name' => $lead['name'],
        'email' => $lead['email'],
        'phone' => $lead['phone'],
        'comments' => $lead['comments'],
        'priority' => $lead['priority'],
        'status' => $lead['status']
      );

      if ($assigned_fields === TRUE) {
        $arr_entry['rep_id'] = $lead['uid'];
      }

      $leads[] = $arr_entry;
    }

    echo acrocrm_leads_generate_leads_list_html($leads);
  }
  else {
    echo '<li class="list-group-item lead-list-error-box">No Leads Found</li>';
  }
}

/*
 * Create lead edit modal/pop-up HTML.
 */
function acrocrm_leads_edit_lead_ajax($lead_id, $redirect_page) {
  $form = drupal_get_form('acrocrm_leads_edit_lead_form');

  $lead = db_select('leads', 'l')
    ->fields('l')
    ->condition('lead_id', $lead_id, '=')
    ->execute()
    ->fetchAssoc();

  $form['firstname']['#value'] = $lead['firstname'];
  $form['name']['#value'] = $lead['name'];
  $form['email']['#value'] = $lead['email'];
  $form['phone']['#value'] = $lead['phone'];
  $form['comments']['#value'] = $lead['comments'];
  $form['priority']['#value'] = $lead['priority'];
  $form['redirect_page']['#value'] = $redirect_page;

  $assigned_salesrep = db_select('assigned_leads', 'al')
    ->fields('al', array('uid'))
    ->condition('lead_id', $lead_id, '=')
    ->execute()
    ->fetchAssoc();

  if (isset($assigned_salesrep['uid'])) {
    $form['assign']['#value'] = $assigned_salesrep['uid'];
  }

  $form['lead_id']['#value'] = $lead_id;

  $form["#prefix"] =
    '<div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title" id="editLead">Edit Lead</h4>
      </div>
      <div class="modal-body">';

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save Changes',
    '#prefix' => '</div><div class="modal-footer"><button type="button" class="btn btn-default" data-dismiss="modal">Close</button>',
    '#suffix' => '</div>',
    '#weight' => 7,
    '#attributes' => array(
      'class' => array('btn', 'btn-primary'),
    ),
  );

  echo drupal_render($form);
}

function acrocrm_leads_delete_lead_ajax($lead_id, $redirect_page) {
  db_update('leads')
    ->fields(array('status' => 'DELETED'))
    ->condition('lead_id', $lead_id, '=')
    ->execute();

  // TODO: Remove assignments
  // TODO: Somehow we have to be able to recover a deleted lead

  drupal_goto($path = "/acrocrm_leads/" . $redirect_page);
}

/*
 * Create the HTML to display a lead
 *
 * @TODO: This would warrant a template...
 * @TODO: Missing t() on text
 */
function acrocrm_leads_generate_leads_html($leads) {
  $search_div = '';

  $search_div .=
    '<div id="lead-search" class="filter-header" data-url="' . base_path() . 'acrocrm_leads/filter_leads/">
            <button type="button" id="create-new-lead-button" class="btn btn-default" data-tooltip="tooltip" title="Create New Lead" data-url="' . base_path() . 'acrocrm_leads/new_lead/" data-toggle="modal" data-target="#lead-create-modal">
                <i class="glyphicon glyphicon-plus" style="color:#009900;"></i>
            </button>
            <div class="input-group">
                <input type="text" class="form-control" aria-label="..." placeholder="">
                <div class="input-group-btn">
                    <button type="button" id="lead-search-button" class="btn btn-default">
                        <img class="search-spinner" src="' . base_path() . drupal_get_path('module', 'acrocrm') . '/templates/assets/images/spinner.gif"><i class="glyphicon glyphicon-search"></i>
                    </button>
                    <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
                        <span class="caret"></span>
                    </button>
                    <ul class="dropdown-menu dropdown-menu-right" style="z-index: 1;">
                        <li>&nbsp;&nbsp;<b>Search by</b></li>
                        <li><a href="#" class="lead-filter" data-group="search-field" data-value="name">Name</a></li>
                        <li><a href="#" class="lead-filter" data-group="search-field" data-value="email">Email</a></li>
                        <li><a href="#" class="lead-filter" data-group="search-field" data-value="phone">Phone</a></li>
                        <li><a href="#" class="lead-filter" data-group="search-field" data-value="comments">Comments</a></li>

                        <li role="separator" class="divider"></li>

                        <li>&nbsp;&nbsp;<b>Show</b></li>
                        <li><a href="#" class="lead-filter" data-group="show" data-value="all">All</a></li>
                        <li><a href="#" class="lead-filter" data-group="show" data-value="assigned">Assigned</a></li>
                        <li><a href="#" class="lead-filter" data-group="show" data-value="unassigned">Unassigned</a></li>
                        <li><a href="#" class="lead-filter" data-group="show" data-value="deleted">Deleted</a></li>

                        <li role="separator" class="divider"></li>

                        <li>
                            &nbsp;&nbsp;
                            <b>Sort by</b>
                            &nbsp;
                            <span class="btn-group btn-group-xs pull-right" role="group" style="margin-right: 5px;">
                                <button type="button" class="lead-filter btn btn-default" data-group="sort-order" data-value="asc">Asc</button>
                                <button type="button" class="lead-filter btn btn-default" data-group="sort-order" data-value="desc">Desc</button>
                            </span>
                        </li>
                        <li><a href="#" class="lead-filter" data-group="sort" data-value="created_time">Date Created</a></li>
                        <li><a href="#" class="lead-filter" data-group="sort" data-value="modified_time">Date Modified</a></li>
                        <li><a href="#" class="lead-filter" data-group="sort" data-value="priority">Priority</a></li>
                        <li><a href="#" class="lead-filter" data-group="sort" data-value="name">Last Name</a></li>
                        <li><a href="#" class="lead-filter" data-group="sort" data-value="email">Email</a></li>
                    </ul>
                </div>
            </div>
        </div>';

  drupal_session_start();

  // Set defaults
  if (!isset($_SESSION['lead_search_pref'])) {
    $_SESSION['lead_search_pref'] = 'name';
  }

  if (!isset($_SESSION['lead_show_pref'])) {
    $_SESSION['lead_show_pref'] = 'unassigned';
  }

  if (!isset($_SESSION['lead_sort_pref'])) {
    $_SESSION['lead_sort_pref'] = 'created_time';
  }

  if (!isset($_SESSION['lead_sort_order_pref'])) {
    $_SESSION['lead_sort_order_pref'] = 'asc';
  }

  if (!isset($_SESSION['lead_search_val'])) {
    $_SESSION['lead_search_val'] = '';
  }

  // @ TODO: Use drupal_add_js().
  $search_div .=
    '<script id="temporary-lead-dropdown-setup-script">
                $(document).ready(function() {
                    $("ul li a[data-group=\'search-field\'][data-value=\'' . $_SESSION['lead_search_pref'] . '\']").prepend("<i class=\'lead-search-dropdown-check glyphicon glyphicon-ok\'></i>");
                    $("ul li a[data-group=\'show\'][data-value=\'' . $_SESSION['lead_show_pref'] . '\']").prepend("<i class=\'lead-search-dropdown-check glyphicon glyphicon-ok\'></i>");
                    $("ul li a[data-group=\'sort\'][data-value=\'' . $_SESSION['lead_sort_pref'] . '\']").prepend("<i class=\'lead-search-dropdown-check glyphicon glyphicon-ok\'></i>");
                    $("[data-group=\'sort-order\'][data-value=\'' . $_SESSION['lead_sort_order_pref'] . '\']").addClass("active");
                    $("#lead-search input").attr("placeholder", "Search by ' . $_SESSION['lead_search_pref'] . '");
                    $("#leads-list img").hide();

                    $("#temporary-lead-dropdown-setup-script").remove();
                });
            </script>';

  $lead_create_modal =
    '<div class="modal fade" id="lead-create-modal" tabindex="-1" role="dialog" aria-labelledby="new-lead">
			<div class="vertical-alignment-helper">
				<div class="modal-dialog modal-lg vertical-align-center" role="document">
					<div class="modal-content"></div>
				</div>
			</div>
		</div>';

  $leads_edit_modal =
    '<div class="modal fade" id="lead-edit-modal" tabindex="-1" role="dialog" aria-labelledby="edit-lead">
			<div class="vertical-alignment-helper">
				<div class="modal-dialog modal-lg vertical-align-center" role="document">
					<div class="modal-content"></div>
				</div>
			</div>
		</div>';

  $delete_lead_confirmation_modal =
    '<div class="modal fade" id="delete-lead-confirmation-modal" tabindex="-1" role="dialog" aria-labelledby="delete-lead">
			<div class="vertical-alignment-helper">
				<div class="modal-dialog modal-sm vertical-align-center" role="document">
					<div class="modal-content">
						<div class="modal-header">
							<button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
							<h4 class="modal-title" id="edit-lead">Delete Lead</h4>
						</div>
						<div class="modal-body">
							Are you sure you want to delete this lead?
						</div>
						<div class="modal-footer">
							<button type="button" class="btn btn-default" data-dismiss="modal">No</button>
							<button type="button" id="delete-lead-confirmation-button" class="btn btn-danger">Yes</button>
						</div>
					</div>
				</div>
			</div>
		</div>';

  $leads_list = '<ul id="leads-list" class="list-group"></ul>';
  $leads_div = '<div class="col-md-6" style="margin-top: 15px;">' . $search_div . $lead_create_modal . $leads_edit_modal . $delete_lead_confirmation_modal . $leads_list . '</div>';

  return $leads_div;
}

function acrocrm_leads_generate_leads_list_html(&$leads) {
  $leads_list = '';

  foreach ($leads as $lead) {
    if (strlen($lead['firstname']) > 0) {
      $lead_name = $lead['firstname'] . ' ' . $lead['name'];
    }
    else {
      $lead_name = $lead['name'];
    }

    $assigned = (isset($lead['rep_id']) && $lead['rep_id'] != NULL) ? TRUE : FALSE;

    if ($assigned === TRUE) {
      $lead_status = 'Assigned';
      $ball_indicator = 'yellow-ball';
    }

    switch ($lead['status']) {
      case 'APPROVED':
        $lead_status = 'Assigned and Approved';
        $ball_indicator = 'green-ball';
        break;
      case 'UNAPPROVED':
        if ($assigned !== TRUE) {
          $lead_status = 'Not Assigned and Not Approved';
          $ball_indicator = 'grey-ball';
        }
        else {
          $lead_status = 'Assigned but Not Approved';
        }
        break;
      case 'DELETED':
        $lead_status = 'Deleted';
        $ball_indicator = 'red-ball';
        break;
      default:
        continue;
    }

    $leads_list .= '<li data-lead-id="' . $lead['lead_id'] . '" class="list-group-item' . (($assigned !== TRUE && $lead_status !== 'Deleted') ? ' unassigned-lead' : '') . '" data-url="' . base_path() . 'acrocrm_leads/" href="javascript:void(0)">';

    $leads_list .= '<div class="pull-right text-right">';
    $leads_list .= '<div class="btn-group" role="group" aria-label="...">';
    $leads_list .= '<button data-tooltip="tooltip" title="Edit Lead" type="button" class="edit-lead-button btn btn-primary" data-url="' . base_path() . 'acrocrm_leads/edit_lead/' . $lead['lead_id'] . '" data-toggle="modal" data-target="#lead-edit-modal"><span class="glyphicon glyphicon-pencil"></span></button>';
    $leads_list .= '<button data-tooltip="tooltip" title="Delete Lead" type="button" class="delete-lead-button btn btn-danger" data-url="' . base_path() . 'acrocrm_leads/delete_lead/' . $lead['lead_id'] . '/overview' . '" data-toggle="modal" data-target="#delete-lead-confirmation-modal"><span class="glyphicon glyphicon-trash"></span></button>';
    $leads_list .= '</div>';
    $leads_list .= '</div>';

    $leads_list .= '<h4 class="list-group-item-heading">';
    $leads_list .= $lead_name;
    $leads_list .= ' <span data-tooltip="tooltip" title="' . $lead_status . '" class="ball-indicator ' . $ball_indicator . '"></span>';
    $leads_list .= '</h4>';

    if (isset($lead['email']) && $lead['email'] != '') {
      $leads_list .= '<p class="list-group-item-text"><strong>Email: </strong>' . $lead['email'] . '</p>';
    }

    if (isset($lead['phone']) && $lead['phone'] != '') {
      $leads_list .= '<p class="list-group-item-text"><strong>Phone: </strong>' . $lead['phone'] . '</p>';
    }

    // TODO: Make priority dropdown into 4 styled radio buttons instead
    $priority = $lead['priority'];
    $leads_list .= '<p class="list-group-item-text"><strong>Priority: </strong>';
    if ($priority === '') {
      $priority = 'unassigned';
    }
    $leads_list .= $priority . '</p>';
//        $leads_list .= '<div class="priority-radio">';
//        $leads_list .= '<input type="radio"><div class="check"></div>';
//        $leads_list .= '<input type="radio">';
//        $leads_list .= '<input type="radio">';
//        $leads_list .= '<input type="radio">';
//        $leads_list .= '</div>';
    if (isset($lead['comments']) && $lead['comments'] != '') {
      $leads_list .= '<p class="list-group-item-text readmore"><strong>Comments: </strong>' . $lead['comments'] . '</p>';
    }
    $leads_list .= '</li>';
  }

  return $leads_list;
}

/*
 * Create the HTML to display a lead on the my leads page.
 */
// TODO: Try to amalgamate this function into acrocrm_leads_generate_leads_html as it is mostly redundant and old code
function acrocrm_leads_my_leads_generate_leads_html($leads, $search_bar, $page_name) {
  $search_div = '';

  if ($search_bar) {
    $search_div =
      '<div id="lead-search">
                <div class="input-group">
                    <input type="text" class="form-control" aria-label="..." placeholder="Search by name">
                    <div class="input-group-btn">
                        <button type="button" id="lead-search-button" class="btn btn-default" data-url="' . base_path() . 'acrocrm_leads/search_leads/">
                            <i class="glyphicon glyphicon-search"></i>
                        </button>
                        <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
                            <span class="caret"></span>
                        </button>
                        <ul class="dropdown-menu dropdown-menu-right" data-url="' . base_path() . 'acrocrm_leads/filter_leads/" style="z-index: 1;">
                            <li>&nbsp;&nbsp;<b>Search by</b></li>
                            <li><a href="#" class="lead-filter" data-group="search" data-value="name">Name</a></li>
                            <li><a href="#" class="lead-filter" data-group="search" data-value="email">Email</a></li>

                            <li role="separator" class="divider"></li>

                            <li>&nbsp;&nbsp;<b>Show</b></li>
                            <li><a href="#" class="lead-filter" data-group="show" data-value="all"><i class="glyphicon glyphicon-ok"></i> All</a></li>
                            <li><a href="#" class="lead-filter" data-group="show" data-value="unassigned">Unassigned</a></li>
                            <li><a href="#" class="lead-filter" data-group="show" data-value="deleted">Deleted</a></li>

                            <li role="separator" class="divider"></li>

                            <li>&nbsp;&nbsp;<b>Sort by</b></li>
                            <li><a href="#" class="lead-filter" data-group="sort" data-value="name">Name</a></li>
                            <li><a href="#" class="lead-filter" data-group="sort" data-value="email">Email</a></li>
                        </ul>
                    </div>
                </div>
            </div>';
  }

  $leads_edit_modal =
    '<div class="modal fade" id="lead-edit-modal" tabindex="-1" role="dialog" aria-labelledby="edit-lead">
        <div class="vertical-alignment-helper">
          <div class="modal-dialog modal-lg vertical-align-center" role="document">
            <div class="modal-content">
            </div>
          </div>
        </div>
      </div>';

  $delete_lead_confirmation_modal =
    '<div class="modal fade" id="delete-lead-confirmation-modal" tabindex="-1" role="dialog" aria-labelledby="delete-lead">
        <div class="vertical-alignment-helper">
          <div class="modal-dialog modal-sm vertical-align-center" role="document">
            <div class="modal-content">
              <div class="modal-header">
                <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
                <h4 class="modal-title" id="edit-lead">Delete Lead</h4>
              </div>
              <div class="modal-body">
                Are you sure you want to delete this lead?
              </div>
              <div class="modal-footer">
                <button type="button" class="btn btn-default" data-dismiss="modal">No</button>
                <button type="button" id="delete-lead-confirmation-button" class="btn btn-danger">Yes</button>
              </div>
            </div>
          </div>
        </div>
      </div>';

  $leads_list = '';

  if (count($leads) > 0) {
    foreach ($leads as $lead) {
      $leads_list .= '<div id="' . $lead['lead_id'] . '">';

      $leads_list .= '<div class="pull-right text-right">';
      $leads_list .= '<div class="btn-group" role="group" aria-label="...">';
      if (module_exists('acrocrm_hubspot_integration') && $page_name == 'my_leads') {
        $leads_list .= '<button id="' . $lead['lead_id'] . '" type="button" class="btn btn-success" onclick="createHubspotContact(' . $lead['lead_id'] . ', \'' . base_path() . '\')"><span class="glyphicon glyphicon-user"></span></button>';
      }
      $leads_list .= '<button type="button" class="btn btn-primary" data-url="' . base_path() . 'acrocrm_leads/edit_lead/' . $lead['lead_id'] . '/my_leads' . '" data-toggle="modal" data-target="#lead-edit-modal"><span class="glyphicon glyphicon-pencil"></span></button>';
      $leads_list .= '<button type="button" class="btn btn-danger" data-url="' . base_path() . 'acrocrm_leads/delete_lead/' . $lead['lead_id'] . '/my_leads' . '" data-toggle="modal" data-target="#delete-lead-confirmation-modal"><span class="glyphicon glyphicon-trash"></span></button>';
      $leads_list .= '</div>';


      $priority = $lead['priority'];
      $leads_list .= '<div class="priority"><strong>Priority: </strong>';
      $leads_list .= '<select class="set-priority input-group" onchange="assignPriority(' . $lead['lead_id'] . ', this)">';
      $leads_list .= '<option value="unassigned"';
      if ($priority === '') {
        $leads_list .= ' selected ';
      }
      $leads_list .= '>unassigned</option>';
      $leads_list .= '<option value="low"';
      if ($priority === 'low') {
        $leads_list .= ' selected ';
      }
      $leads_list .= '>low</option>';
      $leads_list .= '<option value="medium"';
      if ($priority === 'medium') {
        $leads_list .= ' selected ';
      }
      $leads_list .= '>medium</option>';
      $leads_list .= '<option value="high"';
      if ($priority === 'high') {
        $leads_list .= ' high ';
      }
      $leads_list .= '>high</option>';
      $leads_list .= '</select></div>';

      $leads_list .= '</div>';

      if (strlen($lead['firstname']) > 0) {
        $leads_list .= '<h4 class="list-group-item-heading">' . $lead['firstname'] . ' ' . $lead['name'] . '</h4>';
      }
      else {
        $leads_list .= '<h4 class="list-group-item-heading">' . $lead['name'] . '</h4>';
      }

      $leads_list .= '<p class="list-group-item-text"><strong>Email: </strong>' . $lead['email'] . '</p>';
      $leads_list .= '<p class="list-group-item-text"><strong>Phone: </strong>' . $lead['phone'] . '</p>';
      $leads_list .= '<p class="list-group-item-text"><strong>Comments: </strong>' . $lead['comments'] . '</p>';

      $leads_list .= '</div>';
    }
  }
  else {
    $leads_list = '<div class="no-leads">There are no leads to display.</div>';
  }
  $leads_div = ($page_name == 'my_leads' ? '<div id="leads-list-my-leads" class="list-group col-md-12">' : '<div id="leads-list" class="list-group col-md-6 drag-list">') . $search_div . $leads_edit_modal . $delete_lead_confirmation_modal . $leads_list . '</div>';

  return $leads_div;
}

/*
 * Get a list of all sales reps and create html to display sales reps
 */
function acrocrm_leads_get_sales_reps() {
  // TODO: Add buttons/dropdowns and functionality to the filter bar. Need date range, collapse all, open all, send to hubspot, email, etc.
  $filter_bar =
    '<div id="lead-search" class="filter-header" data-url="' . base_path() . 'acrocrm_leads/filter_leads/">
            <div class="input-group">'
    . (module_exists('acrocrm_hubspot_integration') && variable_get('automatic_creation') == TRUE ? '<button id="commit-leads-button" type="button" class="btn btn-primary" data-module-base-path="' . drupal_get_path('module', 'acrocrm') . '" data-url-base-path="' . base_path() . '">Commit Assigned Leads</button>' : '') .
    // TODO: Add functions for the buttons below.
//        '<div class="btn-group" role="group" aria-label="...">
//                    <button type="button" class="btn btn-default">1</button>
//                    <button type="button" class="btn btn-default">2</button>
//
//                    <div class="btn-group" role="group">
//                        <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
//                            Dropdown
//                            <span class="caret"></span>
//                        </button>
//                        <ul class="dropdown-menu">
//                            <li><a href="#">Dropdown link</a></li>
//                            <li><a href="#">Dropdown link</a></li>
//                        </ul>
//                    </div>
//                </div>
    '</div>
        </div>';

  $users = entity_load('user');
  $rep_div = '';

  foreach ($users as $user) {
    if (user_access('sales rep', $user)) {
      $leads_list = '';
      $priorities = acrocrm_leads_generate_sales_rep_lead_list($user->uid, $leads_list);

      $rep_div .= '<li data-rep-id="' . $user->uid . '" class="list-group-item sales-rep">';
      $rep_div .= '    <h4 class="list-group-item-heading  sorting-disabled sales-rep-closed">' . $user->name;
      $rep_div .= '        <span class="pull-right">';
      $rep_div .= '           <span data-tooltip="tooltip" title="High Priority" id="high-' . $user->uid . '" class="high-priority priority-indicator">' . $priorities['high'] . '</span>';
      $rep_div .= '           <span data-tooltip="tooltip" title="Medium Priority" id="med-' . $user->uid . '" class="med-priority priority-indicator">' . $priorities['med'] . '</span>';
      $rep_div .= '           <span data-tooltip="tooltip" title="Low Priority" id="low-' . $user->uid . '" class="low-priority priority-indicator">' . $priorities['low'] . '</span>';
      $rep_div .= '           <span data-tooltip="tooltip" title="Undefined Priority" id="unassigned-' . $user->uid . '" class="priority-indicator">' . $priorities['unassigned'] . '</span>';
      $rep_div .= '        </span>';
      $rep_div .= '    </h4>';

      $rep_div .= '    <div>';
      $rep_div .= '        <ul class="list-group sales-rep-lead-list">';
      $rep_div .= $leads_list;
      $rep_div .= '        </ul>';
      $rep_div .= '    </div>';
      $rep_div .= '</li>';
    }
  }

  $sales_reps = '<div class="col-md-6" style="margin-top: 15px;">' . $filter_bar . '<ul class="list-group sales-rep-list">' . $rep_div . '</ul></div>';
  return $sales_reps;
}

function acrocrm_leads_generate_sales_rep_lead_list($sales_rep_user_id, &$leads_list) {
  $leads_list = '';

  $assigned_leads = db_select('assigned_leads', 'lead_id')
    ->fields('lead_id')
    ->condition('uid', $sales_rep_user_id, '=')
    ->execute();

  $lead_ids = array();

  while ($lead_id = $assigned_leads->fetchAssoc()) {
    array_push($lead_ids, $lead_id["lead_id"]);
  }

  // keep track of the priority for each type of lead -> will be displayed next to rep's name
  $priorities['unassigned'] = 0;
  $priorities['low'] = 0;
  $priorities['med'] = 0;
  $priorities['high'] = 0;

  if (count($lead_ids) > 0) {
    $leads = db_select('leads', 'l')
      ->fields('l')
      ->condition('lead_id', $lead_ids, 'IN')
      // TODO: Might need to do this a better way
      ->condition('status', array('APPROVED', 'UNAPPROVED'), 'IN')
      ->orderBy('priority', 'ASC')
      ->execute();

    if ($leads->rowCount() > 0) {
      while ($lead = $leads->fetchAssoc()) {
        $leads_list .= '<li data-lead-id="' . $lead['lead_id'] . '" data-assigned-rep-id="' . $sales_rep_user_id . '" class="' . (($lead['status'] !== 'APPROVED') ? 'unapproved-lead ' : '') . 'list-group-item sales-rep-lead" data-url="' . base_path() . 'acrocrm_leads/">';

        $leads_list .= '<div class="pull-right text-right">';

        if ($lead['status'] !== 'APPROVED') {
          $leads_list .= '<div class="btn-group" role="group" aria-label="...">';
          $leads_list .= '<button data-tooltip="tooltip" title="Edit Lead" type="button" class="edit-lead-button btn btn-xs btn-primary" data-url="' . base_path() . 'acrocrm_leads/edit_lead/' . $lead['lead_id'] . '" data-toggle="modal" data-target="#lead-edit-modal"><span class="glyphicon glyphicon-pencil"></span></button>';
          $leads_list .= '<button data-tooltip="tooltip" title="Delete Lead" type="button" class="delete-lead-button btn btn-xs btn-danger" data-url="' . base_path() . 'acrocrm_leads/delete_lead/' . $lead['lead_id'] . '/overview' . '" data-toggle="modal" data-target="#delete-lead-confirmation-modal"><span class="glyphicon glyphicon-trash"></span></button>';
          $leads_list .= '</div>';
        }
        else {
          $leads_list .= '<span data-tooltip="tooltip" title="Approved" class="ball-indicator green-ball"></span>';
        }

        $leads_list .= '</div>';
        if (strlen($lead['firstname']) > 0) {
          $leads_list .= '<h5>' . $lead['firstname'] . ' ' . $lead['name'];
        }
        else {
          $leads_list .= '<h5><span class="accordion-title">' . $lead['name'];
        }
        $leads_list .= '</h5>';

        // start accordion div
        $leads_list .= '<div>';

        if (isset($lead['email']) && $lead['email'] != '') {
          $leads_list .= '<p class="list-group-item-text"><strong>Email: </strong>' . $lead['email'] . '</p>';
        }

        if (isset($lead['phone']) && $lead['phone'] != '') {
          $leads_list .= '<p class="list-group-item-text"><strong>Phone: </strong>' . $lead['phone'] . '</p>';
        }

        // determine what the priority of the lead is so we can show it in the rep div
        $priority = $lead['priority'];
        if ($priority == '') {
          $priorities['unassigned'] += 1;
        }
        else {
          if ($priority == 'low') {
            $priorities['low'] += 1;
          }
          else {
            if ($priority == 'medium') {
              $priorities['med'] += 1;
            }
            else {
              if ($priority == 'high') {
                $priorities['high'] += 1;
              }
            }
          }
        }

        // TODO: Make priority dropdown into 4 styled radio buttons instead
        $leads_list .= '<p class="list-group-item-text"><strong>Priority: </strong>';
        if ($priority === '') {
          $priority = 'unassigned';
        }
        $leads_list .= $priority . '</p>';

        if (isset($lead['comments']) && $lead['comments'] != '') {
          $leads_list .= '<p class="list-group-item-text readmore"><strong>Comments: </strong>' . $lead['comments'] . '</p>';
        }
        $leads_list .= '</div>';

        $leads_list .= '</li>';
      }
    }
    else {
      $leads_list .= '<li class="list-group-item no-assigned-leads">No Assigned Leads</li>';
    }
  }
  else {
    $leads_list .= '<li class="list-group-item no-assigned-leads">No Assigned Leads</li>';
  }

  return $priorities;
}

function acrocrm_leads_my_leads_page() {
  $page['content']['header'] = array(
    '#markup' => '<div id="header-row" class="row">
                    <div class="col-lg-12">
                        <h1 class="page-header">My Leads</h1>
                    </div>
                  </div>'
  );
  // Need to initialize a placeholder so this displays before the page content.
  $page['content']['messages']['#markup'] = '';

  global $user;

  $prefix = '<div class="row">
                    <div class="col-lg-10 col-md-12">
                            <div class="panel panel-default">
                                <div class="panel-heading">Currently Assigned Leads</div>
                                <div class="panel-body">';
  $suffix = '</div></div></div></div></div>';

  $page['content']['leads$'] = array(
    '#markup' => $prefix . acrocrm_leads_my_leads_generate_leads_html(acrocrm_leads_assigned_leads($user->uid),
        FALSE,
        'my_leads') . $suffix
  );

  // The messages appear after the form has been loaded.
  $page['content']['messages']['#markup'] = acrocrm_get_message_markup();

  return theme('acrocrm_leads_home', array('page' => $page));
}

function acrocrm_leads_new_lead_ajax() {
  $form = drupal_get_form('acrocrm_leads_new_lead_form');

  $form["#prefix"] =
    '<div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title" id="editLead">Create New Lead</h4>
      </div>
      <div class="modal-body">';

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Create Lead',
    '#prefix' => '</div><div class="modal-footer"><button type="button" class="btn btn-default" data-dismiss="modal">Close</button>',
    '#suffix' => '</div>',
    '#weight' => 7,
    '#attributes' => array(
      'class' => array('btn', 'btn-primary'),
    ),
  );

  echo drupal_render($form);
}

function acrocrm_leads_edit_lead_form($form, &$form_state) {
  generate_lead_form($form, $form_state);
  return $form;
}

function acrocrm_leads_new_lead_form($form, &$form_state) {
  generate_lead_form($form, $form_state);
  return $form;
}

function generate_lead_form(&$form, &$form_state) {
  $form['priority'] = array(
    '#type' => 'select',
    '#title' => t('Priority'),
    '#options' => array(
      'high' => t('High'),
      'medium' => t('Medium'),
      'low' => t('Low'),
    ),
    '#required' => FALSE,
    '#weight' => 0,
    '#empty_value' => '',
    '#description' => t('The priority of the lead.'),
    '#prefix' => '<div class="row"><div class="col-lg-6 col-md-6"><div class="form-group">',
    '#suffix' => '</div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  /* Create list of users who are sales representatives. */
  $users = entity_load('user');
  $sales_reps = array();

  foreach ($users as $user) {
    if (user_access('sales rep', $user)) {
      $sales_reps[$user->uid] = $user->name;
    }
  }

  $form['assign'] = array(
    '#type' => 'select',
    '#title' => t('Assign to'),
    '#options' => $sales_reps,
    '#required' => FALSE,
    '#weight' => 1,
    '#empty_value' => '',
    '#description' => t('The sales rep which the lead should be assigned to.'),
    '#prefix' => '<div class="col-lg-6 col-md-6"><div class="form-group">',
    '#suffix' => '</div></div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  $form['firstname'] = array(
    '#type' => 'textfield',
    '#title' => t('First Name'),
    '#size' => 50,
    '#maxlength' => 50,
    '#required' => FALSE,
    '#weight' => 2,
    '#prefix' => '<div class="row"><div class="col-lg-6"><div class="form-group">',
    '#suffix' => '</div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Last Name'),
    '#size' => 50,
    '#maxlength' => 50,
    '#required' => TRUE,
    '#weight' => 3,
    '#prefix' => '<div class="col-lg-6"><div class="form-group">',
    '#suffix' => '</div></div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  $form['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
    '#size' => 50,
    '#maxlength' => 50,
    '#required' => TRUE,
    '#weight' => 4,
    '#prefix' => '<div class="row"><div class="col-lg-6"><div class="form-group">',
    '#suffix' => '</div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  $form['phone'] = array(
    '#type' => 'textfield',
    '#title' => t('Phone Number'),
    '#size' => 32,
    '#maxlength' => 15,
    '#required' => FALSE,
    '#weight' => 5,
    '#prefix' => '<div class="col-lg-6"><div class="form-group">',
    '#suffix' => '</div></div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  $form['comments'] = array(
    '#type' => 'textarea',
    '#title' => t('Comments'),
    '#maxlength' => 1000,
    '#required' => TRUE,
    '#weight' => 6,
    '#prefix' => '<div class="row"><div class="col-lg-12"><div class="form-group">',
    '#suffix' => '</div></div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  $form['lead_id'] = array(
    '#type' => 'hidden',
  );

  $form['redirect_page'] = array(
    '#type' => 'hidden',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#prefix' => '<div class="row"><div class="col-lg-12"><div class="form-group">',
    '#suffix' => '</div></div></div>',
    '#weight' => 7,
    '#attributes' => array(
      'class' => array('btn', 'btn-primary'),
    ),
  );
}

/**
 * Submit callback for acrocrm_leads_edit_lead_form().
 */
function acrocrm_leads_edit_lead_form_submit($form, $form_state) {
  db_update('leads')
    ->fields(array(
      'name' => $form_state['values']['name'],
      'firstname' => $form_state['values']['firstname'],
      'email' => $form_state['values']['email'],
      'phone' => $form_state['values']['phone'],
      'comments' => $form_state['values']['comments'],
      'priority' => $form_state['values']['priority']
    ))
    ->condition('lead_id', $form_state['values']['lead_id'], '=')
    ->execute();

  $check_exists = db_select('assigned_leads', 'al')
    ->fields('al')
    ->condition('lead_id', $form_state['values']['lead_id'], '=')
    ->execute();

  if ($form_state['values']['assign'] != 0) {
    if ($check_exists->rowCount() > 0) {
      db_update('assigned_leads')
        ->fields(array(
          'uid' => $form_state['values']['assign']
        ))
        ->condition('lead_id', $form_state['values']['lead_id'], '=')
        ->execute();
    }
    else {
      db_insert('assigned_leads')
        ->fields(array(
          'uid' => $form_state['values']['assign'],
          'lead_id' => $form_state['values']['lead_id']
        ))
        ->execute();
    }
  }
  else {
    if ($check_exists->rowCount() > 0) {
      db_delete('assigned_leads')
        ->condition('lead_id', $form_state['values']['lead_id'])
        ->execute();
    }
  }

  drupal_set_message(t('The lead has been updated successfully.'));
}

/**
 * Submit callback for acrocrm_leads_new_lead_form().
 */
function acrocrm_leads_new_lead_form_submit($form, $form_state) {
  $lead_id = db_insert('leads')
    ->fields(array(
      'name' => $form_state['values']['name'],
      'firstname' => $form_state['values']['firstname'],
      'email' => $form_state['values']['email'],
      'phone' => $form_state['values']['phone'],
      'comments' => $form_state['values']['comments'],
      'priority' => $form_state['values']['priority']
    ))
    ->execute();

  if ($form_state['values']['assign'] != '') {
    db_insert('assigned_leads')
      ->fields(array(
        'uid' => $form_state['values']['assign'],
        'lead_id' => $lead_id,
      ))
      ->execute();
  }

  drupal_set_message(t('The new lead has been created successfully.'));
}

/**
 * Implements validation from the Form API for the new lead form.
 *
 * @param $form
 *   A structured array containing the elements and properties of the form.
 * @param $form_state
 *   An array that stores information about the form's current state
 *   during processing.
 */
function acrocrm_leads_new_lead_form_validate($form, &$form_state) {
  generate_lead_validation($form, $form_state);
}

/**
 * Implements validation from the Form API for the edit lead form.
 *
 * @param $form
 *   A structured array containing the elements and properties of the form.
 * @param $form_state
 *   An array that stores information about the form's current state
 *   during processing.
 */
function acrocrm_leads_edit_lead_form_validate($form, &$form_state) {
  generate_lead_validation($form, $form_state);
}

function generate_lead_validation(&$form, &$form_state) {
  $email = $form_state['values']['email'];
  $email_pattern = '/^[^@]*@[^@]*/';
  if ($email != '' && !preg_match($email_pattern, $email)) {
    form_set_error('contact_email', t('You must enter a valid email address.'));
  }
}
