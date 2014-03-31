<?php

/**
 * @file
 * Defines a form for managing an objects collaborators.
 */

/**
 * Form definition function for the select collaborators ingest form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function saskdora_select_collaborators_form(array $form, array &$form_state) {
  global $user;
  form_load_include($form_state, 'inc', 'saskdora', 'includes/select_collaborators.form');
  module_load_include('inc', 'saskdora', 'includes/utilities');
  drupal_set_title(t('Select Collaborators'));
  // Identify who really owns this object.
  $object = islandora_ingest_form_get_object($form_state);
  $investigator = saskdora_get_primary_investigator($object);
  $form_state['primary_investigator_name'] = $investigator ? $investigator->name : saskdora_get_primary_investigator_name($object);
  // Collaborators are stored in the form state and manipulated via AJAX queries
  // rather than using the actual collaborators tableselect as a value directly.
  $default_collaborators = ($user->name == $form_state['primary_investigator_name']) ? array() : array($user->name);
  $form_state['collaborators'] = isset($form_state['collaborators']) ?
    $form_state['collaborators'] :
    $default_collaborators;
  $options = array();
  foreach ($form_state['collaborators'] as $collaborator) {
    $account = user_load_by_name($collaborator);
    $options[$collaborator] = array(
      'user' => array(
        'data' => array(
          '#markup' => l($account->name, "user/{$account->uid}"),
        ),
      ),
      'mail' => array(
        'data' => array(
          '#markup' => l($account->mail, "mailto:{$account->mail}"),
        ),
      ),
    );
  }
  $investigator_markup = $investigator ? l($investigator->name, "user/{$investigator->uid}") : $form_state['primary_investigator_name'];
  return array(
    '#prefix' => '<div id="select-collaborator-form-wrapper">',
    '#suffix' => '</div>',
    'primary' => array(
      '#type' => 'item',
      '#title' => t('Primary Investigator (Owner)'),
      '#markup' => $investigator_markup,
    ),
    'collaborators' => array(
      '#theme_wrappers' => array('form_element'),
      '#title' => t('Collaborators'),
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
      '#submit' => array('saskdora_select_collaborators_form_remove_selected_collaborators'),
      '#ajax' => array(
        'wrapper' => 'select-collaborator-form-wrapper',
        'callback' => 'saskdora_select_collaborators_form_ajax_callback',
        'method' => 'replace',
      ),
      '#limit_validation_errors' => array(
        array('collaborators'),
      ),
    ),
    'collaborator' => array(
      '#type' => 'textfield',
      '#title' => t('Username'),
      '#description' => t('Give a new user collaborator access.'),
      '#default_value' => NULL,
      '#autocomplete_path' => 'user/autocomplete',
      '#element_validate' => array('saskdora_select_collaborators_form_collaborator_validate'),
    ),
    'add' => array(
      '#type' => 'submit',
      '#value' => t('Add'),
      '#submit' => array('saskdora_select_collaborators_form_add_collaborator'),
      '#limit_validation_errors' => array(
        array('collaborator'),
      ),
      '#ajax' => array(
        'wrapper' => 'select-collaborator-form-wrapper',
        'callback' => 'saskdora_select_collaborators_form_ajax_callback',
        'method' => 'replace',
      ),
    ),
    'br' => array(
      '#markup' => '<br/>',
    ),
  );
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function saskdora_form_saskdora_select_collaborators_form_alter(array &$form, array &$form_state) {
  // Limit validation errors on next.
  $form['next']['#limit_validation_errors'] = array(
    array('collaborators'),
  );
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
function saskdora_select_collaborators_form_collaborator_validate(array $element, array &$form_state, array $form) {
  if (empty($element['#value'])) {
    form_error($element, t('No user specified'));
  }
  elseif ($element['#value'] == $form_state['primary_investigator_name']) {
    form_error($element, t('User %user is the Primary Investigator, and is assumed to have collaborator access.', array('%user' => $element['#value'])));
  }
  elseif (in_array($element['#value'], $form_state['collaborators'])) {
    form_error($element, t('User %user is already a collaborator', array('%user' => $element['#value'])));
  }
  elseif (user_load_by_name($element['#value']) === FALSE) {
    form_error($element, t('The given user %user does not exist', array('%user' => $element['#value'])));
  }
}

/**
 * Returns the select collaborators form to be re-rendered.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function saskdora_select_collaborators_form_ajax_callback(array $form, array $form_state) {
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
function saskdora_select_collaborators_form_remove_selected_collaborators(array $form, array &$form_state) {
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
function saskdora_select_collaborators_form_add_collaborator(array $form, array &$form_state) {
  $form_state['collaborators'][] = $form_state['values']['collaborator'];
  $form_state['rebuild'] = TRUE;
}

/**
 * The submit callback for the select collaborators form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function saskdora_select_collaborators_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'saskdora', 'includes/utilities');
  $object = islandora_ingest_form_get_object($form_state);
  saskdora_set_collaborators($object, $form_state['collaborators']);
}
