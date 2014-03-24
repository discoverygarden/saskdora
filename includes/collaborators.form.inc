<?php

/**
 * @file
 * Defines a form for managing an objects collaborators.
 */

/**
 * Form definition function for the collaborators form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $object
 *   The object whose collaborators this form alters.
 *
 * @return array
 *   The Drupal form definition.
 */
function saskdora_manage_collaborators_form(array $form, array &$form_state, AbstractObject $object) {
  module_load_include('inc', 'saskdora', 'includes/utilities');
  drupal_set_title(t('Manage Collaborators'));
  $form_state['object'] = $object->id;
  $collaborators = saskdora_get_collaborators($object);
  $form_state['collaborators'] = isset($form_state['collaborators']) ?
      $form_state['collaborators'] :
      $collaborators;
  $options = array();
  foreach ($form_state['collaborators'] as $collaborator) {
    $account = user_load_by_name($collaborator);
    if ($account === FALSE) {
      continue;
    }
    $options[$collaborator] = array(
      'user' => array(
        'data' => array(
          '#markup' => $collaborator,
        ),
      ),
      'mail' => array(
        'data' => array(
          '#markup' => $account->mail,
        ),
      ),
    );
  }
  $messages = saskdora_manage_collaborators_form_get_pending_changes_messages($collaborators, $form_state['collaborators']);
  return array(
    '#prefix' => '<div id="manage-collaborator-form-wrapper">',
    '#suffix' => '</div>',
    'changed' => array(
      '#access' => count($messages),
      '#theme_wrappers' => array('container'),
      '#attributes' => array('class' => array('view-changed', 'messages', 'warning')),
      '#markup' => theme('item_list', array(
        'type' => 'ul',
        'items' => $messages,
      )),
    ),
    'collaborators' => array(
      '#type' => 'tableselect',
      '#header' => array(
        'user' => array('data' => t('User')),
        'mail' => array('data' => t('Email')),
      ),
      '#options' => $options,
      '#empty' => t('No Collaborators'),
    ),
    'remove' => array(
      '#type' => 'submit',
      '#value' => t('Remove Selected'),
      '#submit' => array('saskdora_manage_collaborators_form_remove_selected_collaborators'),
      '#ajax' => array(
        'wrapper' => 'manage-collaborator-form-wrapper',
        'callback' => 'saskdora_manage_collaborators_form_ajax_callback',
        'method' => 'replace',
      ),
      '#limit_validation_errors' => array(
        array('collaborators'),
      ),
    ),
    'collaborator' => array(
      '#type' => 'textfield',
      '#title' => t('Username'),
      '#default_value' => NULL,
      '#autocomplete_path' => 'user/autocomplete',
      '#element_validate' => array('saskdora_manage_collaborators_form_collaborator_validate'),
    ),
    'add' => array(
      '#type' => 'submit',
      '#value' => t('Add'),
      '#submit' => array('saskdora_manage_collaborators_form_add_collaborator'),
      '#limit_validation_errors' => array(
        array('collaborator'),
      ),
      '#ajax' => array(
        'wrapper' => 'manage-collaborator-form-wrapper',
        'callback' => 'saskdora_manage_collaborators_form_ajax_callback',
        'method' => 'replace',
      ),
    ),
    'recursive' => array(
      '#type' => 'radios',
      '#title' => t('Apply changes to children'),
      '#required' => TRUE,
      '#options' => array(
        'none' => t('Do not apply changes <small><em>(Only change the current object, do not modify its children)</em></small>'),
        'differences' => t("Only apply differences <small><em>(Add / Remove the collaborator(s) changed but do not remove the children's other collaborators)</em></small>"),
        'identical' => t('Identical <small><em>(Children will have the exact same collaborators as shown)</em></small>'),
      ),

    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save Changes'),
      '#submit' => array('saskdora_manage_collaborators_form_submit'),
      '#limit_validation_errors' => array(
        array('collaborators'),
        array('recursive'),
      ),
    ),
    'cancel' => array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
      '#submit' => array('saskdora_manage_collaborators_form_cancel_submit'),
      '#limit_validation_errors' => array(),
    ),
  );
}

/**
 * A set of messages describing the pending changes.
 *
 * @param array $original_collaborators
 *   The original collaborators.
 * @param array $new_collaborators
 *   The new collaborators, as selected by the user.
 *
 * @return string[]
 *   A number of messages describing the pending changes.
 */
function saskdora_manage_collaborators_form_get_pending_changes_messages(array $original_collaborators, array $new_collaborators) {
  $added = array_diff($new_collaborators, $original_collaborators);
  $removed = array_diff($original_collaborators, $new_collaborators);
  $modified = $removed || $added;
  $messages = array();
  if ($modified) {
    $messages[] = t('* All changes are stored temporarily. Click Save to make your changes permanent. Click Cancel to discard your changes.');
  }
  if (!empty($removed)) {
    $messages[] = t('%users will no longer be able to collaborate', array('%users' => implode(', ', $removed)));
  }
  if (!empty($added)) {
    $messages[] = t('%users will now be able to collaborate', array('%users' => implode(', ', $added)));
  }
  return $messages;
}

/**
 * Ensures the given user exists.
 *
 * @param array $element
 *   The Drupal form element to validate.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $form
 *   The Drupal form definition.
 */
function saskdora_manage_collaborators_form_collaborator_validate(array $element, array &$form_state, array $form) {
  if (empty($element['#value'])) {
    form_error($element, t('No user specified'));
  }
  else if (in_array($element['#value'], $form_state['collaborators'])) {
    form_error($element, t('User %user is already a collaborator', array('%user' => $element['#value'])));
  }
  else if (user_load_by_name($element['#value']) === FALSE) {
    form_error($element, t('The given user %user does not exist', array('%user' => $element['#value'])));
  }
}

/**
 * Returns the collaborators form to be re-rendered.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function saskdora_manage_collaborators_form_ajax_callback(array $form, array $form_state) {
  // Always unset the value in collaborator regardless of the situation. As we
  // don't want the field to stay filled when the form is submitted.
  $form['collaborator']['#value'] = NULL;
  return $form;
}

/**
 * Removes a single collaborator from the form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function saskdora_manage_collaborators_form_remove_selected_collaborators(array $form, array &$form_state) {
  $collaborators = array_filter($form_state['values']['collaborators']);
  $form_state['collaborators'] = array_diff($form_state['collaborators'], $collaborators);
  $form_state['rebuild'] = TRUE;
}

/**
 * Adds a single collaborator specified in the autocomplete field.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function saskdora_manage_collaborators_form_add_collaborator(array $form, array &$form_state) {
  $form_state['collaborators'][] = $form_state['values']['collaborator'];
  $form_state['rebuild'] = TRUE;
}

/**
 * The submit callback to cancel out of the collaborators form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function saskdora_manage_collaborators_form_cancel_submit(array $form, array &$form_state) {
  $form_state['redirect'] = "islandora/object/{$form_state['object']}";
}

/**
 * The submit callback for the collaborators form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function saskdora_manage_collaborators_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'saskdora', 'includes/manage_collaborators.batch');
  $object = islandora_object_load($form_state['object']);
  $original_collaborators = saskdora_get_collaborators($object);
  $new_collaborators = $form_state['values']['collaborators'];
  $added = array_diff($new_collaborators, $original_collaborators);
  $removed = array_diff($original_collaborators, $new_collaborators);
  switch ($form_state['values']['recursive']) {
    case 'none':
      $batch = saskdora_collaborators_destructive_update_batch_definition($object, $new_collaborators);
      break;

    case 'differences':
      $batch = saskdora_collaborators_recursive_nondestructive_update_batch_definition($object, $added, $removed);
      break;

    case 'identical':
      $batch = saskdora_collaborators_recursive_destructive_update_batch_definition($object, $new_collaborators);
      break;
  }
  batch_set($batch);
}
