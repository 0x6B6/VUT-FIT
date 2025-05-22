describe('Calendar reservations', () => {

	// Background
	beforeEach(() => {		
		cy.login('admin', 'admin123')
		cy.clearCustomers()
		cy.visit('/index.php/calendar')
		cy.clearUnavailabilities()

		// Current time
		const time = new Date()
		const day = String(time.getDate()).padStart(2, '0')
		const month = String(time.getMonth() + 1).padStart(2, '0')
		const year = time.getFullYear()

		// Event time range setting
		const from = '9:00 am'
		const to = '9:30 am'

		cy.wrap({ day, month, year, from, to }).as('currentDate')
	})

	// # 10)
	it('Creation of a new appointment reservation', () => {
		cy.get('@currentDate').then(({ day, month, year, from, to }) => {
			// Creates reservation
			cy.createReservation(day, month, year, from, to)

			// Checks if it appears in the calendar
			cy.get(`td[data-date="${year}-${month}-${day}"]`)
			  .find('.fc-timegrid-event-harness')
			  .within(() => {
			    cy.get('.fc-event-main-frame').within(() => {
			      cy.get('.fc-event-time').should('contain', `${from} - ${to}`)
			      cy.get('.fc-event-title.fc-sticky').should('contain', 'Room 42 - John Bar')
			    })
			  })

			// Visits customer page
			cy.visit('/index.php/customers')

			// Searches for Customer and verifies that
			// the reservation appears in the customer's appointments
			cy.get('.results')
			.contains('John Bar')
			.click()

			cy.get('#customer-appointments .appointment-row')
			.contains('Room 42 - Jane Doe')

			cy.get('#customer-appointments .appointment-row')
			.contains(`${day}/${month}/${year} ${from}`)
			
			cy.get('#customer-appointments .appointment-row')
			.contains(`${day}/${month}/${year} ${to}`)
			.should('exist')
		})
	})

	// # 11)
	it('Edit an appointment reservation', () => {
		cy.get('@currentDate').then(({ day, month, year, from, to }) => {
			// Creates reservation
			cy.createReservation(day, month, year, from, to)

			// Clicks edit button
			cy.get('.fc-event-main').click()
			cy.get('.edit-popover').click()

			// Updates date
			cy.get('#start-datetime').clear().type(`${day}/${month}/${year} 10:00 am{enter}`)
			cy.get('#end-datetime').clear().type(`${day}/${month}/${year} 12:30 pm{enter}`)

			// Saves it
			cy.get('#save-appointment').click()

			// Visits customer page
			cy.visit('/index.php/customers')

			// Searches for Customer and verifies that
			// the reservation appears UPDATED in the customer's appointments
			cy.get('.results')
			.contains('John Bar')
			.click()

			cy.get('#customer-appointments .appointment-row')
			.contains('Room 42 - Jane Doe')

			cy.get('#customer-appointments .appointment-row')
			.contains(`${day}/${month}/${year} 10:00 am`)
			
			cy.get('#customer-appointments .appointment-row')
			.contains(`${day}/${month}/${year} 12:30 pm`)
			.should('exist')
		})
	})

	// # 12)
	it('Deletes an appointment reservation', () => {
		cy.get('@currentDate').then(({ day, month, year, from, to }) => {
			// Creates reservation
			cy.createReservation(day, month, year, from, to)

			// Deletes reservation from calendar
			cy.get('.fc-event-main').click()
			cy.get('.delete-popover').click()
			cy.intercept('POST', '/index.php/calendar/get_calendar_appointments').as('loadAppointments')
			cy.get('#message-modal > .modal-dialog > .modal-content > .modal-footer > .btn-primary').click()

			// Checks that it has been removed from the calendar
			cy.wait('@loadAppointments')

			cy.get('.fc-scrollgrid').find('.fc-event-main').should('not.exist')

			// Visits customer page
			cy.visit('/index.php/customers')

			// Checks that reservation no longer appears in the customer's appointments
			cy.get('.results')
				.contains('John Bar')
				.click()

			cy.get('#customer-appointments').contains('No records found...')
		})
	})

	// # 13)
	it('Pick existing customer during appointment creation', () => {
		// Populate customers
		cy.visit('/index.php/customers')

		cy.createCustomer('customer')
		cy.createCustomer('customer3')

		cy.visit('/index.php/calendar')

		// Click the add appointment button
		cy.get('.dropdown.d-sm-inline-block button.btn.btn-light[data-bs-toggle="dropdown"]').click()
		cy.get('#insert-appointment').click()

		// Click the 'select customer' button 
		cy.get('#select-customer').click()

		// Customer list with existing customers should appear
		cy.get('#existing-customers-list').contains('John Bar')
		cy.get('#existing-customers-list').contains('Gordon Freeman')

		// Non-existing customers should not be in the list
		cy.get('#existing-customers-list').should('not.contain.text', 'Bar Foo')
		cy.get('#existing-customers-list').should('not.contain.text', 'Email Man')
	})

	// # 14)
	it('Create an unavailability', () => {
		cy.get('@currentDate').then(({ day, month, year, from, to }) => {
			// Create unavailability
			cy.createUnavailability(day, month, year, from, to)

			// Checks if its in the calendar in the right date column
			cy.get(`td[data-date="${year}-${month}-${day}"]`)
			  .find('.fc-timegrid-event-harness')
			  .within(() => {
			    cy.get('.fc-event-main-frame').within(() => {
			      cy.get('.fc-event-time').should('contain', `${from} - ${to}`)
			      cy.get('.fc-event-title.fc-sticky').should('contain', 'Unavailability')
			    })
			  })
		})
	})

	// # 15)
	it('Change of appointments customer', () => {
		cy.get('@currentDate').then(({ day, month, year, from, to }) => {
			cy.createReservation(day, month, year, from, to)

			// Clicks edit button
			cy.get('.fc-event-main').click()
			cy.get('.edit-popover').click()

			// Reassignts to a new/other customer
			cy.get('#new-customer').click()

			cy.fillForm('customer3')

			cy.get('#save-appointment').click()

			// Visits customer page
			cy.visit('/index.php/customers')

			// Check that the original customer
			// has had the appointment removed
			cy.get('.results')
			.contains('John Bar')
			.click()

			cy.get('#customer-appointments')
				.should('contain', 'No records found...')

			// Check that the new customer
			// has the appointment assigned 
			cy.get('.results')
			.contains('Gordon Freeman')
			.click()

			cy.get('#customer-appointments .appointment-row')
				.should('contain', 'Room 42 - Jane Doe')
				.should('contain', `${day}/${month}/${year} ${from}`)
				.should('contain', `${day}/${month}/${year} ${to}`)
		})
	})

	// # 16)
	it('Creation of a customer via appointment management', () => {
		cy.get('@currentDate').then(({ day, month, year, from, to }) => {
			// Create reservation mockup
			cy.createReservation(day, month, year, from, to)

			// Visits customer page
			cy.visit('/index.php/customers')

			// Verifies that the customer exists,
			// when created from appointment reservation
			cy.get('.results')
			.contains('John Bar')
			.click()

			// Verify the existence of customer
			// created by editing the reservation
			// (reassign)

			// Visits calendar page
			cy.visit('/index.php/calendar')

			// Clicks edit button
			cy.get('.fc-event-main').click()
			cy.get('.edit-popover').click()			

			cy.fillForm('customer3')

			// Saves it
			cy.get('#save-appointment').click()

			// Visits customer page
			cy.visit('/index.php/customers')

			// Verifies that the customer exists,
			// when created from reservation edit
			cy.get('.results')
			.contains('Gordon Freeman')
			.click()			
		})
	})

	// # 24)
	it('Delete an unavailability', () => {
		cy.get('@currentDate').then(({ day, month, year, from, to }) => {
			// Create unavailability
			cy.createUnavailability(day, month, year, from, to)

			// Deletes unavailability from calendar
			cy.get('.fc-event-main').click()
			cy.intercept('POST', '/index.php/calendar/get_calendar_appointments').as('loadAppointments')
			cy.get('.delete-popover').click()

			// Wait for the calendar to refresh
			cy.wait('@loadAppointments')

			// Checks that it has been removed from the calendar
			cy.get('.fc-scrollgrid').find('.fc-event-main').should('not.exist')			
		})		
	})

	// # 25)
	it('Edit an unavailability', () => {
		cy.get('@currentDate').then(({ day, month, year, from, to }) => {
			// Create unavailability
			cy.createUnavailability(day, month, year, from, to)

			// Clicks edit button
			cy.get('.fc-event-main').click()
			cy.get('.edit-popover').click()
			
			// Fill in date
			cy.get('#unavailability-start').clear().type(`${day}/${month}/${year} 10:00 am{enter}`)
			cy.get('#unavailability-end').clear().type(`${day}/${month}/${year} 12:00 pm{enter}`)

			// Save and confirm the unavailability
			cy.get('#save-unavailability').click()

			// Checks if its in the calendar in the right date column
			cy.get(`td[data-date="${year}-${month}-${day}"]`)
			  .find('.fc-timegrid-event-harness')
			  .within(() => {
			    cy.get('.fc-event-main-frame').within(() => {
			      cy.get('.fc-event-time').should('contain', `10:00 am - 12:00 pm`)
			      cy.get('.fc-event-title.fc-sticky').should('contain', 'Unavailability')
			    })
			  })
		})	
	})
})