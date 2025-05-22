Feature: Management of existing customers

  Background:
    Given user is an administrator
    And is in the backend section
    And is on the Customers page
  
  # 17)
  Scenario: Customer creation
    Given admin clicks the "Add" button
    And fills in mandatory information
    When admin clicks the "Save" button
    Then a customer is added to the customer list

  # 18)
  Scenario: Edit customer
    Given a specific customer exists 
    And admin selects the customer from the customer list
    And clicks on the "Edit" button
    And updates the customers details with other valid data
    When admin click the "Save" button
    Then the updated customer entry should be displayed

  # 19)
  Scenario: Customer deletion
    Given a specific customer exists
    And admin selects the customer from the customer list
    And clicks on the "Delete" button
    When admin confirms the deletion
    Then the customer is removed from the customer list
  
  # 20)
  Scenario Outline: Search for customer
    Given a specific customer exists
    When admin searches for the customer using <information>
    Then filtered customer list containg the customer is displayed
    But the customer not matching the <information> is not displayed
    
    Examples:
    | information         |
    | Gordon              |
    | Gordon Freeman      |
    | gordon@testco.local |
    | +1 (000) 000-0000   |

  # 21)
  Scenario Outline: Search for non existent customer
    Given the customer does not exist
    And no existing customer entries match <information>
    When admin searches for <information>
    Then no customers are listed in the customer list 
    And "No records found..." is displayed in the search result 

  Examples:
  | information        |
  | Error              |
  | Error Error        |
  | error@test.local   |
  | + 0 (999) 999-9999 |

  # 22)
  Scenario: Edit in customers appointment list
    Given a specific customer exists
    And the customer has an appointment reservation
    When admin clicks on the "Edit" button
    Then admin is redirected to the Calendar page
    And the Edit Appointment window is displayed for the appointment

  # 23)
  Scenario: Adding a customer with already used Email
    Given a customer with <email> already exists
    And  admin is creating a new customer with the <email>
    When admin clicks on the "Save" button
    Then the customer is not created
    And "The provided email address is already in use, please use a different one." message is displayed