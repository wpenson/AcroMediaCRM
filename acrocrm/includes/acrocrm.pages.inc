<?php

function acrocrm_landing_page() {
    if (module_exists('acrocrm_dashboard')) {
        drupal_goto($path = "/acrocrm_dashboard");
    } elseif (module_exists('acrocrm_leads')) {
        drupal_goto($path = "/acrocrm_leads/overview");
    } else {
        drupal_goto($path = "/acrocrm/default");
    }
}

function acrocrm_default() {
    $page['content'] = array(
        '#markup' => '<h2>There are no modules installed.</h2>'
    );
    return theme('acrocrm_home', array('page' => $page));
}