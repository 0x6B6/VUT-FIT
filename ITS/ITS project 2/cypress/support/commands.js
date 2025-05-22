Cypress.Commands.add('login', (username, password) => {
	cy.visit('/index.php')
	cy.wait(1000)
		
	cy.get('.backend-link').click()
		
	cy.get('#username').type(username)
	cy.get('#password').type(password)

	cy.get('#login').click()

	cy.url().should('include', '/calendar')
})

Cypress.Commands.add('fillForm', (fixture) => {
	cy.fixture(fixture).then((customer) => {
		cy.get('#first-name').clear().type(customer.first_name)
		cy.get('#last-name').clear().type(customer.last_name)
		cy.get('#email').clear().type(customer.email)
		cy.get('#phone-number').clear().type(customer.phone_number)
	})
})

Cypress.Commands.add('createReservation', (day, month, year, from, to) => {
	// Clicks the add appointment button
	cy.get('.dropdown.d-sm-inline-block button.btn.btn-light[data-bs-toggle="dropdown"]').click()
	cy.get('#insert-appointment').click()

	// Fills reservation form
	cy.fillForm('customer')

	// Fill in date
	cy.get('#start-datetime').clear().type(`${day}/${month}/${year} ${from}{enter}`)
	cy.get('#end-datetime').clear().type(`${day}/${month}/${year} ${to}{enter}`)

	// Save and confirm the appointment reservation
	cy.get('#save-appointment').click()
})

Cypress.Commands.add('createUnavailability', (day, month, year, from, to) => {
	// Clicks the add unavailability button
	cy.get('.dropdown.d-sm-inline-block button.btn.btn-light[data-bs-toggle="dropdown"]').click()
	cy.get('#insert-unavailability').click()

	// Select provider
	cy.get('#unavailability-provider').select('Jane Doe')

	// Fill in date
	cy.get('#unavailability-start').clear().type(`${day}/${month}/${year} ${from}{enter}`)
	cy.get('#unavailability-end').clear().type(`${day}/${month}/${year} ${to}{enter}`)

	// Save and confirm the unavailability
	cy.get('#save-unavailability').click()
})

Cypress.Commands.add('clearUnavailabilities', () => {
	// Get all unavailability events from the calendar
	// and delete them
	cy.get('#calendar .fc-event').each(($unavailEvent) => {
	  if ($unavailEvent.text().includes('Unavailability')) {
	  	// Clicks and deletes the unavailability
	    cy.wrap($unavailEvent).click()
	    cy.get('.delete-popover').click()
	  }
	})
})

Cypress.Commands.add('clearCustomers', () => {
	// Visits customer page
	cy.visit('/index.php/customers')

	// Recursive helper function to delete every customer,
	// which also results in deleting of all appointments
	// and effectively clearing the database.
	// Must be recursive, since it *would break* the DOM if done
	// using iterations.
	function deleteAllCustomers() {
		cy.wait(500)

		cy.get('#filter-customers').then(($list) => {
			if ($list.find('.customer-row').length === 0) {
			    return
			}

			cy.get('.customer-row').first().click()
			cy.intercept('POST', '/index.php/customers/search').as('loadCustomers')
			cy.get('#delete-customer').click()
			cy.get('.modal-footer > .btn-primary').click()

			// Waits for the list to update itself
			cy.wait('@loadCustomers')

			deleteAllCustomers()
		})
	}

	deleteAllCustomers()
})

Cypress.Commands.add('createCustomer', (fixture) => {
	cy.get('#add-customer').click()
	cy.fillForm(fixture)
	cy.get('#save-customer').click()
})


Cypress.Commands.add('filterCustomers', (filter, total) => {
	// Filters by given filter string
	cy.get('.key').clear().type(filter)
	cy.get('.filter').click()

	// 'total' number of matches should be found and should
	// contain 'filter' string
	cy.get('#filter-customers .customer-row')
		.should('have.length', total)
		.each(($customerRow) => {
		cy.wrap($customerRow).should('contain.text', filter)
  	})
})