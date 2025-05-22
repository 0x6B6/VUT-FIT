# Test report

- **Author:** Marek Pazúr (xpazurm00)
- **Date:** 2025-05-01
- **Project:** 2. project ITS 2024/25
- **Goal:** Create automated tests in Cypress based on BDD scenarios

## Description of test modifications

- The number of tests has not changed.
- Tests in cypress files have slightly modified names compared to their gherkin counterparts
- Minor adjustments have been made to a few existing tests

Here are listed individual tests that *differ* **in Cypress**, or have been *modified* compared to their *Gherkin Scenarios*.

### book_appointment.cy.js - unregistered_user_reservation.feature

**Scenario: Only future time slots for selected date are displayed**
  - The time is not fixed at 12:00 pm, but is chosen dynamically according to the current time.

**Scenario Outline: Invalid information**
  - Invalid input data slightly differ in Cypress

**Scenario: Proceed to the Appointment Confirmation**

**Scenario: Completing the reservation**

**Scenario: Going back to the booking page for a new appointment**
  - These scenarios were merged into a single test, as each subsequent test merely extended the previous one without any changes. The new scenario covers the entire incremental logic without duplication.

### customers.cy.js - customers.feature

**Scenario Outline: Search for customer**
  - The test was expanded to include searching a list with several users present.

### calendar_reservation.cy.js - calendar_reservation.feature

**Scenario Outline: Pick existing customer**

**Scenario Outline: Creation of a customer for an appointment**
  - Unlike these Gherkin Scenarios, the tests only apply to the booking creation mode, as the form is exactly the same for the editing mode.


## Mapping BDD scenarios to source code

| Scenario file                         | Line | Cypress file               | Line |
|---------------------------------------|------|----------------------------|------|
| unregistered_user_reservation.feature | 8    | book_appointment.cy.js     | 14   |
| unregistered_user_reservation.feature | 17   | book_appointment.cy.js     | 64   |
| unregistered_user_reservation.feature | 26   | book_appointment.cy.js     | 89   |
| unregistered_user_reservation.feature | 33   | book_appointment.cy.js     | 152  |
| unregistered_user_reservation.feature | 47   | book_appointment.cy.js     | 182  |
| unregistered_user_reservation.feature | 59   | book_appointment.cy.js     | 218  |
| unregistered_user_reservation.feature | 75   | book_appointment.cy.js     | 261  |
| unregistered_user_reservation.feature | 83   | book_appointment.cy.js     | 261  |
| unregistered_user_reservation.feature | 89   | book_appointment.cy.js     | 261  |
| calendar_reservation.feature          | 10   | calendar_reservation.cy.js | 24   |
| calendar_reservation.feature          | 18   | calendar_reservation.cy.js | 61   |
| calendar_reservation.feature          | 27   | calendar_reservation.cy.js | 99   |
| calendar_reservation.feature          | 35   | calendar_reservation.cy.js | 128  |
| calendar_reservation.feature          | 47   | calendar_reservation.cy.js | 154  |
| calendar_reservation.feature          | 55   | calendar_reservation.cy.js | 172  |
| calendar_reservation.feature          | 63   | calendar_reservation.cy.js | 213  |
| calendar_reservation.feature          | 75   | calendar_reservation.cy.js | 255  |
| calendar_reservation.feature          | 81   | calendar_reservation.cy.js | 274  |
| customers.feature                     | 10   | customers.cy.js            | 11   |
| customers.feature                     | 17   | customers.cy.js            | 26   |
| customers.feature                     | 26   | customers.cy.js            | 51   |
| customers.feature                     | 34   | customers.cy.js            | 70   |
| customers.feature                     | 48   | customers.cy.js            | 90   |
| customers.feature                     | 63   | customers.cy.js            | 102  |
| customers.feature                     | 71   | customers.cy.js            | 140  |

## Issues

There is a rare problem where the web application does not load in time and a *race condition* may occur, causing a test to return a false negative. However, efforts have been made to fix this problem.
