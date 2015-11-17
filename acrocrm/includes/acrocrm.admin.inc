<?php

/**
 * @file
 *
 */

/**
 * Wrapper for system_admin_menu_block_page().
 */
function acrocrm_landing_page() {
    $path = drupal_get_path('module', 'acrocrm');

    $page['content']['logo'] = array(
        '#theme' => 'image',
        '#path' => $path . '/templates/assets/images/smiley_face.png',
        '#alt' => t('Smile :)'),
        '#attributes' => array('id' => 'smiley-face'),
    );

    /* Create list of users who are sales representatives. */
    $users = entity_load('user');
    $sales_reps = array();

    foreach ($users as $user) {
        if (user_access('sales rep', $user)) {
          $sales_reps[] = $user->name . ', ' . $user->mail;
        }
    }

    $page['content']['sales_reps'] = array(
      '#theme' => 'item_list',
      '#title' => t('Sales Representatives (don\'t know where to put this so it can go here for now)'),
      '#items' => $sales_reps,
      '#cache' => array(
          'keys' => array('acrocrm_sales_rep', 'sales_rep_list'),
          'bin' => 'cache',
          'expire' => time() + 30,
          'granularity' => DRUPAL_CACHE_PER_PAGE,
      ),
    );


  return theme('acrocrm_home', array('page' => $page));
}

/**
 * Settings form for Acro CRM.
 *
 * @return array
 *   System settings form array.
 */
function acrocrm_settings() {
  $form['intro'] = array(
    '#type' => 'item',
    '#markup' => t('Acro CRM settings.'),
  );
  $form['acrocrm_admin_path'] = array(
    '#title' => t('Treat Acro CRM paths as administrative'),
    '#type' => 'checkbox',
    '#description' => t('This is used by other modules to, for example, use the admin theme on Acro CRM paths.'),
    '#default_value' => variable_get('acrocrm_admin_path', TRUE),
  );

  // Allow other modules to inject their own settings.
  $form += module_invoke_all('acrocrm_settings');
  return system_settings_form($form);
}
