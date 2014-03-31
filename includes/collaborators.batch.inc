<?php

/**
 * @file
 * Defines batch functions for manipulating an objects collaborators.
 */

/**
 * Gets a batch definition which will set the given objects collaborators.
 *
 * @param AbstractObject $object
 *   The object to update.
 * @param array $collaborators
 *   The collaborators to set the object to.
 *
 * @return array
 *   The batch definition.
 */
function saskdora_set_collaborators_batch_definition(AbstractObject $object, array $collaborators) {
  return array(
    'operations' => array(
      array('saskdora_collaborators_start_batch_operation', array(array())),
      array('saskdora_collaborators_set_collaborators_batch_operation',
        array($object->id, $collaborators),
      ),
    ),
    'finished' => 'saskdora_collaborators_batch_finished',
    'title' => t('Granting access to @label ...', array('@label' => $object->label)),
    'init_message' => t('Preparing to grant access to @label ...', array('@label' => $object->label)),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'saskdora') . '/includes/collaborators.batch.inc',
  );
}

/**
 * Batch definition which will recursively update an object's collaborators.
 *
 * The batch is non-destructive in that the child objects will not lose any
 * collaborators in which are not referenced in $remove.
 *
 * @param AbstractObject $object
 *   The object to update.
 * @param array $add
 *   The collaborators to add.
 * @param array $remove
 *   The collaborators to remove.
 *
 * @return array
 *   The batch definition.
 */
function saskdora_update_collaborators_recursive_batch_definition(AbstractObject $object, array $add, array $remove) {
  module_load_include('inc', 'saskdora', 'includes/utilities');
  $descendants = saskdora_get_descendants($object);
  $operations = array(
    array('saskdora_collaborators_start_batch_operation', array(array())),
    array(
      'saskdora_collaborators_update_collaborators_batch_operation',
      array($object->id, $add, $remove),
    ),
  );
  foreach ($descendants as $pid) {
    $operations[] = array(
      'saskdora_collaborators_update_collaborators_batch_operation',
      array($pid, $add, $remove),
    );
  }
  return array(
    'operations' => $operations,
    'finished' => 'saskdora_collaborators_batch_finished',
    'title' => t('Granting access to @label ...', array('@label' => $object->label)),
    'init_message' => t('Preparing to grant access to @label ...', array('@label' => $object->label)),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'saskdora') . '/includes/collaborators.batch.inc',
  );
}

/**
 * Batch definition which will recursively set an object's collaborators.
 *
 * The batch is destructive in that the child objects will loose any
 * collaborators which are not referenced in $collaborators.
 *
 * @param AbstractObject $object
 *   The object to update.
 * @param array $collaborators
 *   The collaborators to grant access to the object.
 *
 * @return array
 *   The batch definition.
 */
function saskdora_set_collaborators_recursive_batch_definition(AbstractObject $object, array $collaborators) {
  module_load_include('inc', 'saskdora', 'includes/utilities');
  $descendants = saskdora_get_descendants($object);
  $operations = array(
    array('saskdora_collaborators_start_batch_operation', array(array())),
    array(
      'saskdora_collaborators_set_collaborators_batch_operation',
      array($object->id, $collaborators),
    ),
  );
  foreach ($descendants as $pid) {
    $operations[] = array(
      'saskdora_collaborators_set_collaborators_batch_operation',
      array($pid, $collaborators),
    );
  }
  return array(
    'operations' => $operations,
    'finished' => 'saskdora_collaborators_batch_finished',
    'title' => t('Granting access to @label ...', array('@label' => $object->label)),
    'init_message' => t('Preparing to grant access to @label ...', array('@label' => $object->label)),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'saskdora') . '/includes/collaborators.batch.inc',
  );
}

/**
 * Batch definition which will recursively set an object's collaborators.
 *
 * The batch is destructive in that the child objects will loose any
 * collaborators which are not referenced in $collaborators.
 *
 * @param AbstractObject $object
 *   The object to update.
 * @param string[] $primary_investigator
 *   The primary investigator.
 * @param string[] $collaborators
 *   The collaborators to grant access to the object.
 *
 * @return array
 *   The batch definition.
 */
function saskdora_set_primary_investigator_and_collaborators_recursive_batch_definition(AbstractObject $object, $primary_investigator, array $collaborators) {
  module_load_include('inc', 'saskdora', 'includes/utilities');
  $descendants = saskdora_get_descendants($object);
  $operations = array(
    array('saskdora_collaborators_start_batch_operation', array(array('display_message' => FALSE))),
    array(
      'saskdora_set_primary_investigator_and_collaborators_batch_operation',
      array($object->id, $primary_investigator, $collaborators),
    ),
  );
  foreach ($descendants as $pid) {
    $operations[] = array(
      'saskdora_set_primary_investigator_and_collaborators_batch_operation',
      array($pid, $primary_investigator, $collaborators),
    );
  }
  return array(
    'operations' => $operations,
    'finished' => 'saskdora_collaborators_batch_finished',
    'title' => t('Granting access to @label ...', array('@label' => $object->label)),
    'init_message' => t('Preparing to grant access to @label ...', array('@label' => $object->label)),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'saskdora') . '/includes/collaborators.batch.inc',
  );
}

/**
 * Sets up a batch by initializing the context.
 *
 * @param array $options
 *   Configuration options for the batch.
 * @param array $context
 *   The context of this batch operation.
 */
function saskdora_collaborators_start_batch_operation(array $options, array &$context) {
  $options += array('display_message' => TRUE);
  $context['results'] = array(
    'options' => $options,
    // "successful" an array of boolean values indicating operation success.
    'successful' => array(),
  );
}

/**
 * Batch operation that updates the POLICY to allow access to the collaborators.
 *
 * This POLICY changes is non-destructive in that other permissions will not be
 * lost.
 *
 * @param string $pid
 *   The PID of the object to modify.
 * @param string[] $add
 *   The collaborators to add to the POLICY.
 * @param string[] $remove
 *   The collaborators to remove from the POLICY.
 * @param array $context
 *   The context of this batch operation.
 */
function saskdora_collaborators_update_collaborators_batch_operation($pid, array $add, array $remove, array &$context) {
  module_load_include('inc', 'saskdora', 'includes/utilities');
  $results = &$context['results'];
  $object = islandora_object_load($pid);
  if (!$object) {
    watchdog('saskdora', 'Object %pid not found and could not be updated.', array('%pid' => $pid), WATCHDOG_ERROR);
    $results['successful'][$pid] = FALSE;
    return;
  }
  try {
    saskdora_update_collaborators($object, $add, $remove);
    watchdog('saskdora', 'Updated collaborators on %pid', array('%pid' => $pid));
    $results['successful'][$pid] = TRUE;
  }
  catch (Exception $e) {
    watchdog_exception('saskdora', $e);
    $results['successful'][$pid] = FALSE;
  }
  $context['message'] = t('Updating @label (@pid)...', array(
    '@pid' => $object->id,
    '@label' => $object->label,
  ));
}

/**
 * Batch operation that updates the POLICY to allow access to the collaborators.
 *
 * This POLICY changes is destructive other permissions will be lost.
 *
 * @param string $pid
 *   The PID of the object to modify.
 * @param string[] $collaborators
 *   The collaborators to set on the POLICY.
 * @param array $context
 *   The context of this batch operation.
 */
function saskdora_collaborators_set_collaborators_batch_operation($pid, array $collaborators, array &$context) {
  module_load_include('inc', 'saskdora', 'includes/utilities');
  $object = islandora_object_load($pid);
  $results = &$context['results'];
  if (!$object) {
    watchdog('saskdora', 'Object %pid not found and could not be updated.', array('%pid' => $pid), WATCHDOG_ERROR);
    $results['successful'][$pid] = FALSE;
    return;
  }
  try {
    saskdora_set_collaborators($object, $collaborators);
    watchdog('saskdora', 'Set collaborators on %pid', array('%pid' => $pid), WATCHDOG_NOTICE);
    $results['successful'][$pid] = TRUE;
  }
  catch (Exception $e) {
    watchdog_exception('saskdora', $e);
    $results['successful'][$pid] = FALSE;
  }
  $context['message'] = t('Updating @label (@pid)...', array(
    '@pid' => $object->id,
    '@label' => $object->label,
  ));
}

/**
 * Batch operation that updates the POLICY to allow access to the collaborators.
 *
 * The Object owner will be set to the given primary investigator.
 *
 * This POLICY changes is destructive other permissions will be lost.
 *
 * @param string $pid
 *   The PID of the object to modify.
 * @param string $primary_investigator
 *   The user name of the primary investigator.
 * @param string[] $collaborators
 *   The collaborators to set on the POLICY.
 * @param array $context
 *   The context of this batch operation.
 */
function saskdora_set_primary_investigator_and_collaborators_batch_operation($pid, $primary_investigator, array $collaborators, array &$context) {
  module_load_include('inc', 'saskdora', 'includes/utilities');
  $results = &$context['results'];
  $object = islandora_object_load($pid);
  if (!$object) {
    watchdog('saskdora', 'Object %pid not found and could not be updated.', array('%pid' => $pid), WATCHDOG_ERROR);
    $results['successful'][$pid] = FALSE;
    return;
  }
  try {
    $object->owner = $primary_investigator;
    saskdora_set_collaborators($object, $collaborators);
    watchdog('saskdora', 'Set collaborators on %pid', array('%pid' => $pid), WATCHDOG_NOTICE);
    $results['successful'][$pid] = TRUE;
  }
  catch (Exception $e) {
    watchdog_exception('saskdora', $e);
    $results['successful'][$pid] = FALSE;
  }
  $context['message'] = t('Updating @label (@pid)...', array(
    '@pid' => $object->id,
    '@label' => $object->label,
  ));
}

/**
 * The batch 'finished' callback.
 */
function saskdora_collaborators_batch_finished($success, $results, $operations) {
  if ($success) {
    if ($results['options']['display_message']) {
      $total = count($results['successful']);
      $successful = count(array_filter($results['successful']));
      $failed = $total - $successful;
      $vars = array(
        '@successful' => $successful,
        '@failed' => $failed,
        '@total' => $total,
      );
      if ($successful) {
        drupal_set_message(t('Successfully updated @successful of @total objects', $vars), 'status');
      }
      if ($failed) {
        drupal_set_message(t('Failed to updated @failed of @total objects, Please please notify the administrator.', $vars), 'error');
      }
    }
  }
  else {
    // Generic Error Message.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array(
      '%error_operation' => $error_operation[0],
      '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}
