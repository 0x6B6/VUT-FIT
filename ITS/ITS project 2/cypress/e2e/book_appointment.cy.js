describe('Appointment page', () => {

  // Background
  beforeEach(() => {
    cy.visit('/index.php/booking')

    cy.wait(1000)

    cy.get('#wizard-frame-2') 
      .should('be.visible')
  })

  // # 1)
  it('Verification of selected appointment time', () => {
    cy.get('#available-hours')
        .find('.available-hour')
        .then(($timeSlots) => {
          // Get number of available appointments
          const count = $timeSlots.length

          // Try to select another timeslot,
          // if possible
          if (count === 1) {
            cy.get('.selected-hour').click()
          } else {
            cy.get('#available-hours > :nth-child(2)').click()
          }

          // Verify that the selected appointment
          // date and time reflects the user's choice
          cy.get('.selected-hour')
              .invoke('text')
              .then((timeslotTime) => {
                cy.get('.flatpickr-day.selected')
                        .invoke('attr', 'aria-label')
                        .then((dateAttr) => {
                           // Get to confirmation
                          cy.get('#button-next-2').click()
                          cy.fillForm('customer3')
                          cy.get('#button-next-3').click()   

                          // Get selected appointment and time
                          cy.get('#appointment-details > :nth-child(1) > :nth-child(3)')
                          .invoke('text')
                          .then((appointmentTime) => {
                            // Convert appointment time to same format
                            const date = new Date(dateAttr)

                            const day = String(date.getDate()).padStart(2, '0')
                            const month = String(date.getMonth() + 1).padStart(2, '0')
                            const year = date.getFullYear()

                            const formattedDate = `${day}/${month}/${year} ${timeslotTime}`

                            // Check that it matches
                            cy.expect(formattedDate).to.equal(appointmentTime.trim())
                          })
                        })
               })
        })
  })

  // # 2)
  it('Displays only future time slots', () => {
    // pick UTC
    // Extract selected date from day picker
    cy.get('.flatpickr-day.selected').invoke('attr','aria-label').then((selectedDate) => {
      // Create a date from it, so it is not dependant on concrete date
      const currentTime = new Date(selectedDate)

      // Get the list of time slot buttons
      cy.get('#available-hours button.btn.btn-outline-secondary').then(($btns) => {
        
        $btns.each((index, btn) => {
          // Convert the timeslot text into proper date format
          const btnDate = `${new Date(selectedDate).toDateString()} ${btn.innerText.trim()}`

          const appointmentTime = new Date(btnDate)

          // Check if every appointment's timeslot is later than current time 
          expect(appointmentTime.getTime()).to.be.greaterThan(currentTime.getTime())
        })
      })

    })
  })

  // # 3)
  it('Reserved time slots are not displayed', () => {
    // Current time
    const time = new Date()

    // Select UTC to match Date()
    cy.get('#select-timezone').select('UTC')

    // Get some timeslot
    cy.get('.selected-hour')
    .invoke('text')
    .then((timeslotTime) => {
      cy.log(timeslotTime)

      // Get available time slots
      cy.get('#available-hours')
        .find('.available-hour')
        .then(($timeSlots) => {
          // Get number of available appointments
          const count = $timeSlots.length

          // Create mock appointment
          cy.get('#button-next-2').click()
          cy.fillForm('customer3')
          cy.get('#button-next-3').click()
          cy.get('#book-appointment-submit').click()
          cy.get('.btn-large').click()

          // Select UTC again to match timezones
          cy.get('#select-timezone').select('UTC')

          // If there are more appointments
          // make sure the reserved appointment
          // is not available in the available appointment's list
          if (count > 1) {
            cy.get('.selected-hour')
            .invoke('text')
            .then((availableTime) => {
              cy.log(availableTime)
              cy.get('#available-hours').should('not.contain', timeslotTime)
            })

          } else {
            // If there is only one appointment available
            // make sure only appointments from the NEXT day
            // are available
            cy.get('.flatpickr-day.selected')
              .invoke('attr', 'aria-label')
              .then((dateAttr) => {
                  const calendarDate = new Date(dateAttr)

                  expect(calendarDate.getTime()).to.be.greaterThan(time.getTime())
              })
          }
        })
      })

      // Teardown
      cy.login('admin','admin123')
      cy.clearCustomers()
      cy.visit('/index.php/booking')
  })

  // # 4)
  it('Not filling in mandatory information', () => {
    // Get to Customer Information wizard frame
    cy.get('#button-next-2')
      .click()

    // Don't fill any fields 
    cy.get('#wizard-frame-3')
      .should('be.visible')

    cy.get('#button-next-3')
      .click()

    // Should stay at the same frame
    cy.get('#wizard-frame-3')
      .should('be.visible')

    cy.get('#wizard-frame-4')
      .should('not.be.visible')

    // Fields should have error messages
    cy.get('#first-name').should('have.class', 'is-invalid')

    cy.get('#last-name').should('have.class', 'is-invalid')

    cy.get('#email').should('have.class', 'is-invalid')

    cy.get('#phone-number').should('have.class', 'is-invalid')
  })

  // # 5)
  it('Filling in invalid information', () => {
    // Get to Customer Information wizard frame    
    cy.get('#button-next-2')
      .click()

    cy.get('#wizard-frame-3')
      .should('be.visible')

    // Fill required fields within the frame with invalid text info 
    cy.get('#first-name')
      .type('John')

    cy.get('#last-name')
      .type('Foo')

    cy.get('#email')
      .type('mail@.com')

    cy.get('#phone-number')
      .type('one two three')

    // Should stay at the same frame
    cy.get('#button-next-3')
      .click()

    cy.get('#wizard-frame-3')
      .should('be.visible')

    cy.get('#wizard-frame-4')
      .should('not.be.visible')

    // Check for email field's error message
    cy.get('#email').should('have.class', 'is-invalid')
  })

  // # 6)
  it('Information preservation when navigating the form', () => {
    // Get to Customer Information wizard frame    
    cy.get('#button-next-2')
      .click()

    cy.get('#wizard-frame-3')
      .should('be.visible')

    // Fill required fields within the frame with valid text info 
    cy.get('#first-name')
      .type('John')

    cy.get('#last-name')
      .type('Foo')

    cy.get('#email')
      .type('johnfoo@mail.com')

    cy.get('#phone-number')
      .type('123-456-789')

    // Go backwards and forwards
    cy.get('#button-back-3')
      .click()

    cy.get('#button-next-2')
      .click()

    // Check field values
    cy.get('#first-name')
      .should('have.value', 'John')

    cy.get('#last-name')
      .should('have.value', 'Foo')

    cy.get('#email')
      .should('have.value', 'johnfoo@mail.com')

    cy.get('#phone-number')
      .should('have.value', '123-456-789')
  })

  // # 7,8,9)
  it('Creates the Appointment and returns to start', () => {
     // Go to Customer Information wizard frame    
    cy.get('#button-next-2')
      .click()
    cy.get('#wizard-frame-3')
      .should('be.visible')

    // Fill required fields within the frame with valid text info 
    cy.get('#first-name')
      .type('John')
    cy.get('#last-name')
      .type('Foo')
    cy.get('#email')
      .type('johnfoo@mail.com')
    cy.get('#phone-number')
      .type('123-456-789')

    // Move to the next page, confirmation frame *should* be displayed
    cy.get('#button-next-3')
      .click()
    cy.get('#wizard-frame-4')
      .should('be.visible')

    // Submit the appointment, succesful registration page should be displayed
    cy.get('#book-appointment-submit')
      .click()

    cy.url().should('include', 'booking_confirmation/of')

    // Given a succesful registration, return back to the booking page 
    cy.get('a[href="http://localhost:8080/index.php"]')
      .click()

    cy.url().should('eq', 'http://localhost:8080/index.php')

    cy.get('#wizard-frame-2') 
      .should('be.visible')
  })

})
