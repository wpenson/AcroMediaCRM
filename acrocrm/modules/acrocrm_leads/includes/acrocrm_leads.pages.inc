<?php

function acrocrm_leads_overview_page() {
    // load the leads from the database
    $leads = acrocrm_leads_load_leads();

    $page['content']['leads'] = array(
        '#markup' => acrocrm_leads_generate_leads_html($leads, true)
    );

    $page['content']['reps'] = array(
        '#markup' => acrocrm_leads_get_sales_reps()
    );

    return theme('acrocrm_leads_home', array('page' => $page));
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
    } else {
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
  } else {
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
  } else {
    $leads_arr[] = 'An error occurred, could not retrieve leads from database.';
  }

  return $leads_arr;
}

/**
 * Gets leads that are assigned to a given user.
 */
function acrocrm_leads_assigned_leads($uid) {
    $leads = db_query('SELECT leads.lead_id, firstname, leads.name AS leadname, email, phone, comments, priority, created_time
                    FROM leads
                    INNER JOIN assigned_leads
                    ON assigned_leads.lead_id = leads.lead_id
                    INNER JOIN users
                    ON assigned_leads.uid = users.uid
                    WHERE users.uid = ' . $uid);

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
  } else {
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
  } else {
    $leads_arr[] = 'An error occurred, could not retrieve leads from database.';
  }

    return $leads_arr;
}

/*
 * Create lead edit modal/pop-up HTML.
 */
function acrocrm_leads_edit_lead_ajax($lead_id) {
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

    $assigned_salesrep = db_select('assigned_leads', 'al')
        ->fields('al', array('uid'))
        ->condition('lead_id', $lead_id, '=')
        ->execute()
        ->fetchAssoc();

    if (isset($assigned_salesrep['uid']))
        $form['assign']['#value'] = $assigned_salesrep['uid'];

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

function acrocrm_leads_delete_lead_ajax($lead_id) {
    db_delete('leads')
        ->condition('lead_id', $lead_id, '=')
        ->execute();

    drupal_goto($path = 'acrocrm_leads/overview');
}

/*
 * Create the HTML to display a lead
 */
function acrocrm_leads_generate_leads_html($leads, $search_bar) {
    $search_div = '';

    if ($search_bar) {
        $search_div =
            '<div class="input-group">
        <input type="text" class="form-control" aria-label="..." placeholder="Search leads">
        <div class="input-group-btn">
          <button type="button" class="btn btn-default">Action</button>
          <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
            <span class="caret"></span>
            <span class="sr-only">Toggle Dropdown</span>
          </button>
          <ul class="dropdown-menu dropdown-menu-right">
            <li><a href="#">Show all</a></li>
            <li><a href="#">Another action</a></li>
            <li><a href="#">Something else here</a></li>
            <li role="separator" class="divider"></li>
            <li><a href="#">Separated link</a></li>
          </ul>
        </div>
      </div>';
    }

    $leads_edit_modal =
      '<div class="modal fade" id="leadEditModal" tabindex="-1" role="dialog" aria-labelledby="editLead">
        <div class="vertical-alignment-helper">
          <div class="modal-dialog modal-lg vertical-align-center" role="document">
            <div class="modal-content">
            </div>
          </div>
        </div>
      </div>';

    $delete_lead_confirmation_modal =
      '<div class="modal fade" id="deleteLeadConfirmationModal" tabindex="-1" role="dialog" aria-labelledby="deleteLead">
        <div class="vertical-alignment-helper">
          <div class="modal-dialog modal-sm vertical-align-center" role="document">
            <div class="modal-content">
              <div class="modal-header">
                <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
                <h4 class="modal-title" id="editLead">Delete Lead</h4>
              </div>
              <div class="modal-body">
                Are you sure you want to delete this lead?
              </div>
              <div class="modal-footer">
                <button type="button" class="btn btn-default" data-dismiss="modal">No</button>
                <button type="button" id="deleteLeadConfirmationButton" class="btn btn-danger">Yes</button>
              </div>
            </div>
          </div>
        </div>
      </div>';

    $leads_list = '';

    foreach ($leads as $lead) {
        $leads_list .= '<div class="list-group-item lead-sortable drag-list" href="javascript:void(0)">';
        $leads_list .= '<div id="lead_' . $lead['lead_id'] . '">';

        $leads_list .= '<div class="pull-right text-right">';
        $leads_list .= '<div class="btn-group" role="group" aria-label="...">';
        $leads_list .= '<button type="button" class="btn btn-primary" data-url="' . base_path() . 'acrocrm_leads/edit_lead/' . $lead['lead_id'] . '" data-toggle="modal" data-target="#leadEditModal"><span class="glyphicon glyphicon-pencil"></span></button>';
        $leads_list .= '<button type="button" class="btn btn-danger" data-url="' . base_path() . 'acrocrm_leads/delete_lead/' . $lead['lead_id'] . '" data-toggle="modal" data-target="#deleteLeadConfirmationModal"><span class="glyphicon glyphicon-trash"></span></button>';
        $leads_list .= '</div>';


        $priority = $lead['priority'];
        $leads_list .= '<div class="priority"><strong>Priority: </strong>';
        $leads_list .= '<select class="set-priority input-group" onchange="assignPriority('. $lead['lead_id'] .', this)">';
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
        } else {
            $leads_list .= '<h4 class="list-group-item-heading">' . $lead['name'] . '</h4>';
        }

        $leads_list .= '<p class="list-group-item-text"><strong>Email: </strong>' . $lead['email'] . '</p>';
        $leads_list .= '<p class="list-group-item-text"><strong>Phone: </strong>' . $lead['phone'] . '</p>';
        $leads_list .= '<p class="list-group-item-text"><strong>Comments: </strong>' . $lead['comments'] . '</p>';

        $leads_list .= '</div></div>';
    }

    $leads_div = '<div id="leads-list" class="list-group col-md-6">' . $search_div . $leads_edit_modal . $delete_lead_confirmation_modal . $leads_list . '</div>';

    return $leads_div;
}

/*
 * Get a list of all sales reps and create html to display sales reps
 */
function acrocrm_leads_get_sales_reps() {
    $users = entity_load('user');

    $rep_div = '';

    foreach ($users as $user) {
        $leads_list = '';

        if (user_access('sales rep', $user)) {
            $assigned_leads = db_select('assigned_leads', 'lead_id')
                ->fields('lead_id')
                ->condition('uid', $user->uid, '=')
                ->execute();

            $lead_ids = array();

            while ($lead_id = $assigned_leads->fetchAssoc()) {
                array_push($lead_ids, $lead_id["lead_id"]);
            }

            if (count($lead_ids) > 0) {
                $leads = db_select('leads', 'l')
                    ->fields('l')
                    ->condition('lead_id', $lead_ids, 'IN')
                    ->execute();

                $leads_list .= '<div id="rep_' . $user->uid . '" class="drag-list lead-list">';

                if ($leads->rowCount() > 0) {
                    while ($lead = $leads->fetchAssoc()) {
                        $leads_list .= '<div id="lead_' . $lead['lead_id'] . '" class="accordion">';
                        if (strlen($lead['firstname']) > 0) {
                            $leads_list .= '<h4>' . $lead['firstname'] . ' ' . $lead['name'];
                            $leads_list .= '<div class="pull-right text-right accordion-btn">';
                            $leads_list .= '<div class="btn-group" role="group" aria-label="...">';
                            $leads_list .= '<button type="button" data-url="' . base_path() . 'acrocrm_leads/edit_lead/' . $lead['lead_id'] . '" class="btn btn-primary" data-toggle="modal" data-target="#leadEditModal"><span class="glyphicon glyphicon-pencil"></span></button>';
                            $leads_list .= '<button type="button" class="btn btn-danger" data-url="' . base_path() . 'acrocrm_leads/delete_lead/' . $lead['lead_id'] . '" data-toggle="modal" data-target="#deleteLeadConfirmationModal"><span class="glyphicon glyphicon-trash"></span></button>';
                            $leads_list .= '</div></div>';
                            $leads_list .= '</h4>';
                        } else {
                            $leads_list .= '<h4>' . $lead['name'] . '</h4>';
                        }
                        // start accordian div
                        $leads_list .= '<div>';
                        $priority = $lead['priority'];
                        $leads_list .= '<div class="priority pull-right rep-priority"><strong>Priority: </strong>';
                        $leads_list .= '<select class="input-group set-priority" onchange="assignPriority('. $lead['lead_id'] .', this)">';
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

                        $leads_list .= '<p class="list-group-item-text"><strong>Email: </strong>' . $lead['email'] . '</p>';
                        $leads_list .= '<p class="list-group-item-text"><strong>Phone: </strong>' . $lead['phone'] . '</p>';
                        $leads_list .= '<p class="list-group-item-text"><strong>Comments: </strong>' . $lead['comments'] . '</p></div>';
                        // end accordian div

                        $leads_list .= '</div>';
                    }
                    $leads_list .= '</div>';
                }
            }
            else {
                $leads_list .= '<div id="rep_' . $user->uid . '" class="drag-list lead-list">';
                $leads_list .= '<h4 class="no-assigned-leads">no assigned leads</h4>';
                $leads_list .= '</div>';
                $leads_list .= '<div class="drag-list lead-list"> </div>';
            }

            $rep_div .= '<div id="' . $user->uid . '" class="list-group-item lead-rep-overview">';
            $rep_div .= '<h4 class="list-group-item-heading">' . $user->name . '</h4>';
            $rep_div .= '<p class="list-group-item-text"><strong>Assigned Leads: </strong></p>';
            $rep_div .= $leads_list;
            $rep_div .= '</div>';
        }
    }

    $sales_reps = '<div class="col-md-6">' . $rep_div . '</div>';

    return $sales_reps;
}

function acrocrm_leads_my_leads_page() {
    $page['content']['header'] = array(
        '#markup' => '<div class="row">
                    <div class="col-lg-12">
                        <h1 class="page-header">My Leads</h1>
                    </div>
                  </div>'
    );
    // Need to initialize a placeholder so this displays before the page content.
    $page['content']['messages']['#markup'] = '';

    global $user;

    $page['content']['leads'] = array(
        '#markup' => acrocrm_leads_generate_leads_html(acrocrm_leads_assigned_leads($user->uid), false)
    );

    // The messages appear after the form has been loaded.
    $page['content']['messages']['#markup'] = acrocrm_get_message_markup();

    return theme('acrocrm_leads_home', array('page' => $page));
}

function acrocrm_leads_new_lead_page() {
    $page['content']['header'] = array(
        '#markup' => '<div class="row">
                    <div class="col-lg-12">
                        <h1 class="page-header">New Lead</h1>
                    </div>
                  </div>'
    );

    // Need to initialize a placeholder so this displays before the form.
    $page['content']['messages']['#markup'] = '';
    $page['content']['new_lead_form'] = drupal_get_form('acrocrm_leads_new_lead_form');

    // The messages appear after the form has been loaded.
    $page['content']['messages']['#markup'] = acrocrm_get_message_markup();

    return theme('acrocrm_leads_home', array('page' => $page));
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
    $form['#prefix'] = '<div class="row">

                        <div class="col-lg-10 col-md-12">
                            <div class="panel panel-default">
                                <div class="panel-heading">Create a New Lead</div>
                                <div class="panel-body">';

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

    $form['#suffix'] = '</div></div></div></div></div>';
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
            'priority' => $form_state['values']['priority']))
        ->condition('lead_id', $form_state['values']['lead_id'], '=')
        ->execute();

    $check_exists = db_select('assigned_leads', 'al')
        ->fields('al')
        ->condition('lead_id', $form_state['values']['lead_id'], '=')
        ->execute();

//    ini_set("log_errors", 1);
//    ini_set("error_log", "/Users/wade/Desktop/php-error.log");
//    error_log(print_r($form_state['values']['lead_id'], $return = true));

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
               'priority' => $form_state['values']['priority']))
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