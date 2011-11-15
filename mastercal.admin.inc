<?php
// $Id: shortcut.admin.inc,v 1.16 2010/10/20 01:31:07 dries Exp $

/**
 * @file
 * Administrative page callbacks
 */





/**
 * Menu page callback: builds the page for administering shortcut sets.
 */
function mastercal_set_admin() {
  $saved_cals = mastercal_load();
  $header = array(t('Name'), array('data' => t('options'), 'colspan' => 4));

  $rows = array();
  foreach ($saved_cals as $cal) {
    $row = array(
      check_plain($cal->title),
      '',
      '',
    );
 
    $row[] = l(t('remove'), "admin/config/user-interface/mastercal/$cal->cid/delete");

    

    $rows[] = $row;
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Form callback: builds the form for adding a shortcut set.
 *
 * @param $form
 *   An associative array containing the structure of the form.
 * @param $form_state
 *   An associative array containing the current state of the form.
 *
 * @return
 *   An array representing the form definition.
 *
 * @ingroup forms
 * @see shortcut_set_add_form_validate()
 * @see shortcut_set_add_form_submit()
 */
function mastercal_set_add_form($form, &$form_state) {

  $calendars = client()->GetCals();
   
   $form['mastercal_name'] = array(
       '#type' => 'select',
       '#title' => t('Calendar'),
       '#options' => $calendars,
       '#default_value' => 0,
       '#description' => t('Master Calendar to add to this site.'),
   );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add Calendar'),
  );

  return $form;
}

/**
 * Validation handler for shortcut_set_add_form().
 */
function mastercal_set_add_form_validate($form, &$form_state) {
  // Check to prevent a duplicate title.
  if (cal_exists($form_state['values']['mastercal_name'])) {
    form_set_error('new', t('The shortcut set %name already exists. Choose another name.', array('%name' => $form_state['values']['mastercal_name'])));
  }
}

/**
 * Submit handler for shortcut_set_add_form().
 */
function mastercal_set_add_form_submit($form, &$form_state) {
  // Save a new shortcut set with links copied from the user's default set.
  $cal = client()->GetCal($form_state['values']['mastercal_name']);
  //print_r($title);
  mastercal_save($cal);
  drupal_set_message(t('The Calendar %cal has been created. You can edit it from this page.', array('%cal' => $cal['Name'])));
  $form_state['redirect'] = 'admin/config/user-interface/mastercal/';
}



/**
 * Form callback: builds the confirmation form for deleting a shortcut set.
 *
 * @param $form
 *   An associative array containing the structure of the form.
 * @param $form_state
 *   An associative array containing the current state of the form.
 * @param object $shortcut_set
 *   An object representing the shortcut set, as returned from
 *   shortcut_set_load().
 *
 * @return
 *   An array representing the form definition.
 *
 * @ingroup forms
 * @see shortcut_set_delete_form_submit()
 */
function mastercal_set_delete_form($form, &$form_state, $cid) {
  $form['cid'] = array(
    '#type' => 'value',
    '#value' => $cid,
  );

  

  return confirm_form(
    $form,
    t('Are you sure you want to delete the shortcut set %title?', array('%title' => $cid)),
    'admin/config/user-interface/mastercal/' . $cid,
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for shortcut_set_delete_form().
 */
function mastercal_set_delete_form_submit($form, &$form_state) {
  $cid = $form_state['values']['cid'];
  mastercal_delete($cid);
  $form_state['redirect'] = 'admin/config/user-interface/mastercal';
  drupal_set_message(t('The shortcut set %title has been deleted.', array('%title' => $cid)));
}


