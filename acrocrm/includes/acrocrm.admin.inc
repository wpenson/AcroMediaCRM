<?php

/**
 * @file
 *
 */

/**
 * Wrapper for system_admin_menu_block_page().
 */
function acrocrm_landing_page() {
  $items = array(
      'settings' => array(
          'path' => 'admin/config/acrocrm',
          'title' => t('Settings'),
          'description' => t('Acro CRM configuration and settings.'),
          'permission' => 'administer acrocrm',
      ),
  );

  foreach ($items as $key => $item) {
    if ((isset($item['module']) && !module_exists($item['module'])) || !user_access($item['permission'])) {
      unset($items[$key]);
    }
  }

  return theme('acrocrm_home', array('items' => $items));
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
