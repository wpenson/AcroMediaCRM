<?php
function acrocrm_hubspot_integration_settings_form() {
  $form['acrocrm_hubspot_integration_hub_id'] = array(
    '#type' => 'textfield',
    '#title' => t('HubSpot Hub ID'),
    '#default_value' => variable_get('acrocrm_hubspot_integration_hub_id'),
    '#description' => t('The HubSpot Hub ID of the account that you want to integrate with.')
  );

  return system_settings_form($form);
}
