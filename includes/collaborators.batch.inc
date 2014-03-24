<?php

/**
 * @file
 *
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
function saskdora_collaborators_destructive_update_batch_definition(AbstractObject $object, array $collaborators) {
  return array(
    'operations' => array(
      array('saskdora_collaborators_start_batch_operation'),
      array('saskdora_collaborators_set_collaborators_batch_operation',
        array($object->id, $collaborators)
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
 * The batch is non-destructive in that the child objects will not loose any
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
function saskdora_collaborators_recursive_nondestructive_update_batch_definition(AbstractObject $object, array $add, array $remove) {
  // @todo Build a query of all the children, take a look at the XACML module.
  return array(
    'operations' => array(
      array(
        'saskdora_collaborators_update_collaborators_batch_operation',
        array($object->id, array('add' => $add, 'remove' => $remove))
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
function saskdora_collaborators_recursive_destructive_update_batch_definition(AbstractObject $object, array $collaborators) {
  // @todo Build a query of all the children, take a look at the XACML module.
  return array(
    'operations' => array(
      array(
        'saskdora_collaborators_set_collaborators_batch_operation',
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
 * Sets up a batch by initializing the context.
 *
 * @param array $context
 *   The context of this batch operation.
 */
function saskdora_collaborators_start_batch_operation(array &$context) {
  // "files" will be deleted upon failure or success of the batch.
  // "successful" an array of boolean values indicating operation success.
  // "messages" array of messages to display on finishing the batch
  // values are used as parameters to t() and drupal_set_message(). That
  // will be displayed when the batch is finished. Placeholders include, but
  // are not limited to: @successful, @failed, @total.
  $context['results'] = array(
    'successful' => array(),
    'messages' => array(),
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
  $object = islandora_object_load($pid);
  if (!$object) {
    $results['successful'][$pid] = FALSE;
    $results['messages'][$pid] = t('Object %pid not found and could not be updated.', array('%pid' => $pid));
    return;
  }
  try {
    saskdora_update_collaborators($object, $add, $remove);
  } catch (Exception $e) {
    $results['successful'][$pid] = FALSE;
    $results['messages'][$pid] = $e->getMessage();
  }
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
  if (!$object) {
    $results['successful'][$pid] = FALSE;
    $results['messages'][$pid] = t('Object %pid not found and could not be updated.', array('%pid' => $pid));
    return;
  }
  try {
    saskdora_set_collaborators($object, $collaborators);
  } catch (Exception $e) {
    $results['successful'][$pid] = FALSE;
    $results['messages'][$pid] = $e->getMessage();
  }
}

/**
 * The batch 'finished' callback.
 */
function saskdora_collaborators_batch_finished($success, $results, $operations) {
  if ($success) {
    // Build generic place holders for the message.
    if (isset($results['successful'])) {
      $total = count($results['successful']);
      $successful = count(array_filter($results['successful']));
      $failed = $total - $successful;
      $place_holders = array(
        '@successful' => $successful,
        '@failed' => $failed,
        '@total' => $total,
      );
      // Display all messages / errors / warnings.
      $messages = array_filter($results['messages']);
      foreach ($messages as $message) {
        $vars = isset($message[1]) ? array_merge($message[1], $place_holders) : $place_holders;
        $status = isset($message[2]) ? $message[2] : 'status';
        $message = format_string($message[0], $vars);
        drupal_set_message(check_plain($message), $status);
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