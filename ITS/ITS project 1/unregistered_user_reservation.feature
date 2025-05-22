Feature: Unregistered user appointment reservation

  Background:
    Given user is not registered
  
  # 1)
  Scenario: User selects appointment date and time
    Given the user is in the Appointment Date and Time section
    And appointment date and time is preselected by default
    When the user selects a different date and time
    And clicks the "Next" button
    Then the user advances to customer information section
    And the selected appointment date and time should reflect the user's choice
  
  # 2)
  Scenario: Only future time slots for selected date are displayed
    Given the user is in the Appointment Date and Time section
    And the local timezone is selected
    And the current time is 12:00 PM
    When the user selects today's date from the calendar
    Then only available time slots after 12:00 PM are displayed
    But the user does not see time slots before 12:00 PM

  # 3)
  Scenario: Reserved time slots are not displayed
    Given the user is in the Appointment Date and Time section
    And a specific time slot is already reserved
    When the user selects the same date as the reserved time slot
    Then the reserved time slot should not be displayed

  # 4)
  Scenario Outline: Filling in mandatory information
    Given the user is in the Customer Information section
    And mandatory <field> information is not filled
    When the user tries to click the "Next" button
    Then the user does not advance to the Appointment Confirmation section

    Examples:
    | field        |
    | First Name   |
    | Last Name    |
    | Email        |
    | Phone Number |
  
  # 5)
  Scenario Outline: Invalid information
    Given the user is in the Customer Information section
    And formatted <field> information is filled with invalid <data>
    When the user tries to click the "Next" button
    Then the user does not advance to the Appointment Confirmation section
    
    Examples:
    | field        | data          |
    | Email        | mail@.com     |
    | Phone Number | one two three |
  
  # 6)
  Scenario: Information preservation when navigating back and forth
    Given the user is in the Customer Information section
    When the user fills in the following fields:
      | First Name   | Gordon                 |
      | Last Name    | Freeman                |
      | Email        | gordonfreeman@mail.com |
      | Phone Number | 123-456-789            |
    And the user clicks the "Back" button
    And the user clicks the "Next" button
    Then the Customer Information section fields should contain the previously filled information:
      | First Name   | Gordon                 |
      | Last Name    | Freeman                |
      | Email        | gordonfreeman@mail.com |
      | Phone Number | 123-456-789            |
  
  # 7)
  Scenario: Proceed to the Appointment Confirmation
    Given the user is in the Customer Information section
    And mandatory information is filled
    And the filled information is valid
    When the user clicks the "Next" button
    Then the user advances to the Appointment Confirmation section
  
  # 8)
  Scenario: Completing the reservation
    Given the user is in the Appointment Confirmation section
    When the user clicks the "Confirm" button
    Then appointment registration confirmation is displayed
  
  # 9)
  Scenario: Going back to the booking page for a new appointment
    Given the user succesfully registered an appointment
    And sees the appointment registration confirmation
    When the user clicks the "Go to booking page" button
    Then the user is redirected to the Appointment and Date section for a new appointment