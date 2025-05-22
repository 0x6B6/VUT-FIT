describe('Customer management', () => {
	
	// Background
	beforeEach(() => {
		cy.login('admin', 'admin123')
		cy.clearCustomers()
		cy.visit('/index.php/customers')
	})

	// # 17)
	it('Creates a customer', () => {
		// Creates a customer
		cy.createCustomer('customer')

		// Waits for the list to update itself
		cy.intercept('POST', '/index.php/customers/search').as('loadCustomers')
		cy.wait('@loadCustomers')

		// Checks that customer is added to the list
		cy.get('.customer-row')
			.should('contain','John Bar')
			.should('contain','johnbar@mail.com, 987-654-321')
	})

	// # 18)
	it('Edits a customer', () => {
		// Creates a customer
		cy.createCustomer('customer')

		// Waits for the list to update itself
		cy.intercept('POST', '/index.php/customers/search').as('loadCustomers')
		cy.wait('@loadCustomers')

		cy.get('.results')
			.contains('John Bar')
			.click()

		cy.get('#edit-customer').click()

		cy.fillForm('customer2')

		cy.get('#save-customer').click()

		cy.get('#first-name').should('have.value', 'Bar')
		cy.get('#last-name').should('have.value', 'Foo')
		cy.get('#email').should('have.value', 'foobar@mail.com')
		cy.get('#phone-number').should('have.value', '123-456-789')
	})

	// # 19)
	it('Deletes a customer', () => {
		// Populates customer list
		cy.createCustomer('customer2')
		cy.createCustomer('customer')

		// Clicks and confirms deletion
		cy.get('#delete-customer').click()
		cy.get('.modal-footer > .btn-primary').click()

		// Waits for the list to update itself
		cy.intercept('POST', '/index.php/customers/search').as('loadCustomers')
		cy.wait('@loadCustomers')

		// Checks that the customer list no longer contains the customer.
		// Email is used to check this, since it serves as an unique ID.
		cy.get('#filter-customers').contains('johnbar@mail.com').should('not.exist')
	})

	// # 20)
	it('Searches for existing customer', () => {
		// Populates customer list
		cy.createCustomer('customer')
		cy.createCustomer('customer2')
		cy.createCustomer('customer3')

		// Find a *single* match of 'Gordon'
		cy.filterCustomers('Gordon', 1)
		cy.filterCustomers('Gordon Freeman', 1)
		cy.filterCustomers('gordon@testco.local', 1)
		cy.filterCustomers('+1 (000) 000-0000', 1)

		// Find *two* matches, both containing 'Bar'
		cy.filterCustomers('Bar', 2)

		// Find a *single* match by unique mail
		cy.filterCustomers('foobar@mail.com', 1)
	})

	// # 21)
	it('Searches for non existing customer', () => {
		// Populates customer list
		cy.createCustomer('customer3')

		// Filter by non existent strings
		cy.filterCustomers('error', 0)
		cy.filterCustomers('Error Error', 0)
		cy.filterCustomers('error@test.local', 0)
		cy.filterCustomers('+ 0 (999) 999-9999', 0)
	})

	// # 22)
	it('Edits appointment from customer page', () => {
		// Current time
		const time = new Date()

		const day = String(time.getDate()).padStart(2, '0')
		const month = String(time.getMonth() + 1).padStart(2, '0')
		const year = time.getFullYear()

		const from = '9:00 am'
		const to = '9:30 am'

		// Creates mock reservation
		cy.visit('/index.php/calendar')
		cy.createReservation(day, month, year, from, to)

		// Returns to customer page
		cy.visit('/index.php/customers')

		// Waits for the list to update itself
		//cy.intercept('POST', '/index.php/customers/search').as('loadCustomers')
		//cy.wait('@loadCustomers')

		// Selects the customer of the appointment
		cy.get('.results')
			.contains('John Bar')
			.click()

		// Clicks on the 'edit appointment' button
		cy.get('.appointment-row > a').click()

		// Should be redirected to calendar page to reschedule the appointment
		cy.url().should('contain', '/calendar/reschedule/')

		// Edit appointment window should be displayed for the appointment
		cy.get('#appointments-modal').should('be.visible')
	})

	// # 23)
	it('Tries to add customer with used email', () => {
		// Create a customer with some email
		cy.createCustomer('customer3')

		// Clicks the 'add customer' button
		cy.get('#add-customer').click()

		// Fill the form with the same email,
		// the rest is not important. 
		cy.fillForm('duplicate3')	

		cy.get('#save-customer').click()

		// Checks if error message has been displayed
		cy.get('#message-modal').should('be.visible')

		cy.get('#message-modal .modal-body')
  		.should('contain.text', 'The provided email address is already in use, please use a different one.')
	})
})