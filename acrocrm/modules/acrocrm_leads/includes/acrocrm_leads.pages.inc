<?php

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

/*
 * Get a list of all leads from the database
 */
function acrocrm_leads_load_leads() {
    $leads = db_query('SELECT * from leads');

    $leads_arr = array();

    if ($leads) {

        foreach ($leads as $lead) {
            $leads_arr[] = array(
                'lead_id' => $lead->lead_id,
                'firstname' => $lead->firstname,
                'name' => $lead->name,
                'email' => $lead->email,
                'phone' => $lead->phone,
                'comments' => $lead->comments,
                'priority' => $lead->priority
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
    $form = drupal_get_form('acrocrm_leads_single_lead_form');

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
            'onclick' => "test_submit();",
            'id' => array('submit'),
        ),
        '#executes_submit_callback' => FALSE,
        //'#submit' => array('acrocrm_leads_single_lead_edit_form_submit'),
        '#ajax' => array(
            'callback' => 'acrocrm_leads_single_lead_edit_form_submit',
            'wrapper' => 'leadEditModal',
            'method' => 'replace',
            'effect' => 'fade',
        ),
        '#id' => 'modalsharebutton',
    );

//    $form['#attached']['library'] = array(
//        array('system', 'drupal.ajax'),
//      array('system', 'jquery.form'),
//    );

//    $page['content']['header'] = array(
//        '#markup' => '<div class="row">
//                    <div class="col-lg-12">
//                        <h1 class="page-header">New Lead</h1>
//                    </div>
//                  </div>'
//    );

    // Need to initialize a placeholder so this displays before the form.
    //$page['content']['messages']['#markup'] = '';
    //$page['content']['new_lead_form'] = $form;

    // The messages appear after the form has been loaded.
    //$page['content']['messages']['#markup'] = acrocrm_get_message_markup();

    echo drupal_render($form);

    //return theme('acrocrm_leads_home', array('page' => $page));
}

/*
 * Create the HTML to display a lead
 */
function acrocrm_leads_generate_leads_html($leads) {

    $search_div = '<div class="input-group">
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

    $leads_list = '';
    $leads_list .= '<a class="btn" href="javascript:void(0)" onclick="myModule_ajax_load()">Ajax Test</a>';
    $leads_list .= '<div id="ajax-target">Ajax goes here!!!</div>';
    foreach ($leads as $lead) {
        //$leads_list .= '<div class="list-group-item lead-sortable drag-list"><div id="lead_' . $lead['lead_id'] . '">';

        $leads_list .= '<div class="list-group-item lead-sortable drag-list" href="javascript:void(0)">';
        $leads_list .= '<div id="lead_' . $lead['lead_id'] . '">';

        $leads_list .= '<div class="pull-right text-right">';
        $leads_list .= '<div class="btn-group" role="group" aria-label="...">';
        $leads_list .= '<button type="button" href="/drupal/acrocrm_leads/edit_lead/' . $lead['lead_id'] . '" class="btn btn-primary" data-id="' . $lead['lead_id'] . '" data-toggle="modal" data-target="#leadEditModal"><span class="glyphicon glyphicon-pencil"></span></button>';
        $leads_list .= '<button type="button" class="btn btn-danger"><span class="glyphicon glyphicon-trash"></span></button>';
        $leads_list .= '</div>';


        $leads_list .= '<div class="priority">Priority:
            <select id="set-priority" class="input-group">
                <option value="unassigned">unassigned</option>
                <option value="low">low</option>
                <option value="medium">medium</option>
                <option value="high">high</option>
            </select>
          </div>';

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

    $leads_div = '<div id="leads-list" class="list-group col-md-6">' . $search_div . $leads_list . '</div>';

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
            $leads_list .= '<ul id="rep_' . $user->uid . '" class="drag-list lead-list">';
            $sql = 'SELECT lead_id from assigned_leads WHERE uid=' . $user->uid;
            $leads =  db_query($sql);

            if ($leads->rowCount() > 0) {
                foreach ($leads as $lead) {
                    $leads_list .= '<li>' . $lead->lead_id . '</li>';
                }
                $leads_list .= '</ul>';
            }
            else {
                $leads_list .= '<li>no assigned leads</li></ul>';
            }

            $rep_div .= '<div id="' . $user->uid . '" class="list-group-item lead-rep-overview" href="javascript:void(0)">';
            $rep_div .= '<h4 class="list-group-item-heading">' . $user->name . '</h4>';
            $rep_div .= '<p class="list-group-item-text"><strong>Assigned Leads: </strong>' . $leads_list . '</p>';
            $rep_div .= '</div>';
        }
    }

    $sales_reps = '<div class="col-md-6">' . $rep_div . '</div>';

    return $sales_reps;
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

function acrocrm_leads_new_lead_form($form, &$form_state) {
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

    return $form;
}

/**
 * Submit callback for acrocrm_leads_new_lead_form().
 */
function acrocrm_leads_single_lead_form_submit($form, $form_state) {
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
    $email = $form_state['values']['email'];
    $email_pattern = '/^[^@]*@[^@]*/';
    if ($email != '' && !preg_match($email_pattern, $email)) {
        form_set_error('contact_email', t('You must enter a valid email address.'));
    }
}