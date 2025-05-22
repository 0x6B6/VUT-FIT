Feature: Management of customer appointment reservations

  Background:
    Given user is an administrator
    And is in the backend section
    And is on the Calendar page
  
  # 10)
  Scenario: Creation of a new appointment reservation
    Given admin opens the New Appointment window
    And fills in valid customer details
    When admin clicks the "Save" button
    Then the newly created reservation should be displayed in the calendar
    And it should appear in the customer's appointments
  
  # 11)
  Scenario: Edit an appointment reservation
    Given admin clicks on an existing appointment in the calendar
    And opens the Edit Appointment window
    And updates the entry details with other valid data
    When admin clicks the "Save" button
    Then the appointment entry should be displayed with updated information
    And it should be updated in the customer's appointments
  
  # 12)
  Scenario: Delete an appointment reservation
    Given admin clicks on a customer's appointment entry
    When the admin clicks the "Delete" button
    And confirms the deletion
    Then the appointment entry is removed from the calendar
    And it should no longer appear in the customer's appointments
  
  # 13)
  Scenario Outline: Pick existing customer
    Given admin opens the <context> window
    And customers exist in customer list
    When admin clicks the "Select" button
    Then list containing existing customers is displayed
  
  Examples:
    | context          |
    | New Appointment  |
    | Edit Appointment |
 
  # 14)
  Scenario: Creation of an unavailability
    Given admin opens the New Unavailability window
    And sets a date
    And sets a time interval
    When admin clicks the "Save" button
    Then the unavailability is displayed in the calendar

  # 15)
  Scenario: Change of appointment's customer
    Given admin opens the Edit Appointment window
    And selects a different customer for the appointment
    When admin clicks the "Save" button
    Then the appointment no longer appears in the original customer's appointments
    And it should appear in the new customer's appointments

  # 16)
  Scenario Outline: Creation of a customer for an appointment
    Given admin opens the <context> window
    And fills in valid customer details of a new customer
    When admin clicks the "Save" button
    Then the newly created customer should be added to the customers list

    Examples:
    | context          |
    | New Appointment  |
    | Edit Appointment |

  # 24)
  Scenario: Delete an unavailability
    Given admin clicks on the unavailability entry
    When the admin clicks the "Delete" button
    Then the unavailability is removed from the calendar

  # 25)
  Scenario: Edit an unavailability
    Given admin clicks on the unavailability entry
    And the admin clicks the "Edit" button
    And the admin updates the entry
    When the admin clicks the "Save" button
    Then the unavailability should be displayed with updated information